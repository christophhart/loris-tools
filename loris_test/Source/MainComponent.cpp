#include "MainComponent.h"

using namespace juce;

//==============================================================================
MainComponent::MainComponent():
  fc("File", File(), true, false, false, "*", "", "Select audio file to analyse"),
  compileButton("Compile"),
  resetButton("Reset")
{
    log.setMultiLine(true);
    log.setFont(GLOBAL_MONOSPACE_FONT());
    addAndMakeVisible(log);
    
    setLookAndFeel(&laf);
    laf.setDefaultColours(fc);
    addAndMakeVisible(fc);
    addAndMakeVisible(compileButton);
    addAndMakeVisible(resetButton);
    
    addAndMakeVisible(commandOptions);
    
    
    
    addAndMakeVisible(commandInput);
    
    
    laf.setDefaultColours(*this);
    commandInput.setLookAndFeel(&laf);
    laf.setTextEditorColours(commandInput);
    
    auto currentDir = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getParentDirectory();
    
    currentDir = currentDir.getParentDirectory();
    currentDir = currentDir.getParentDirectory();
    currentDir = currentDir.getParentDirectory();
    currentDir = currentDir.getParentDirectory();
    currentDir = currentDir.getParentDirectory();
    
    
    DBG(currentDir.getFullPathName());
    
    lorisManager = new LorisManager(currentDir, [&](String error_)
    {
        log.moveCaretToEnd(false);
        error = error_;
        
        log.moveCaretToEnd(false);
        log.insertTextAtCaret("\nERROR: " + error);
        
        repaint();
    });
    
    lorisManager->setLogFunction([&](String m)
    {
        log.moveCaretToEnd(false);
        log.insertTextAtCaret("\n" + m);
    });
    
    commandOptions.addItemList(lorisManager->getList(false), 1);
    
    timeDomain.addItemList({"seconds", "samples", "0to1"}, 1);
    
    timeDomain.onChange = [&]()
    {
        lorisManager->set("time_domain_type", timeDomain.getText());
    };
    
    addAndMakeVisible(timeDomain);
    
    addAndMakeVisible(thumbnail);
    
    fc.addListener(this);
    
    commandInput.setFont(GLOBAL_MONOSPACE_FONT());
    
    
    compileButton.onClick = [&]()
    {
        auto fullCode = commandInput.getText();
        
        bool setMode = false;
        
        if(commandOptions.getText() == "custom")
        {
            juce::JavascriptEngine engine;
            
            lorisManager->processCustom(fc.getCurrentFile(), [&](LorisManager::CustomPOD& data)
            {
                auto obj = data.toJSON();
                
                engine.registerNativeObject("obj", obj.getDynamicObject());
                
                auto ok = engine.execute(fullCode);
                
                if(ok)
                {
                    data.writeJSON(obj);
                    return false;
                }
                else
                {
                    lorisManager->errorFunction(ok.getErrorMessage());
                    return true;
                }
            });
                      
            auto bl = lorisManager->synthesise(fc.getCurrentFile());
            thumbnail.setBuffer(bl[0], bl[1]);
            repaint();
            
            return;
        }
        
        if(fullCode.startsWith("set"))
        {
            setMode = true;
            fullCode = fullCode.fromFirstOccurrenceOf("set", false, false);
        }
        
        auto command = commandOptions.getText();
        auto json = fullCode;
        
        if(setMode)
            lorisManager->set(command, json);
        else
        {
            lorisManager->process(fc.getCurrentFile(), command, json);
            
            auto bl = lorisManager->synthesise(fc.getCurrentFile());
            thumbnail.setBuffer(bl[0], bl[1]);
        }
        
        repaint();
    };
    
    resetButton.onClick = [&]()
    {
        
        lorisManager->process(fc.getCurrentFile(), "reset", "{}");
        log.setText("");
        auto bl = lorisManager->synthesise(fc.getCurrentFile());
        thumbnail.setBuffer(bl[0], bl[1]);
        
        repaint();
    };
    
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colour(0xff222222));

    g.setFont (GLOBAL_FONT());
    g.setColour (juce::Colours::white);
    
    g.drawText (error, errorArea, juce::Justification::centred, true);
}

void MainComponent::resized()
{
    auto b = getLocalBounds();
    
    fc.setBounds(b.removeFromTop(30).reduced(3));
    
    errorArea = b.removeFromTop(50).toFloat();
    
    auto commandRow = b.removeFromTop(40).reduced(3);
    
    resetButton.setBounds(commandRow.removeFromLeft(90));
    commandOptions.setBounds(commandRow.removeFromLeft(100));
    
    compileButton.setBounds(commandRow.removeFromRight(120));
    timeDomain.setBounds(commandRow.removeFromRight(90));
    
    commandInput.setBounds(commandRow);
    
    b.removeFromTop(10);
    
    log.setBounds(b.removeFromBottom(200));
    
    thumbnail.setBounds(b.reduced(3));
    
    
    
    
    
    // This is called when th MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
