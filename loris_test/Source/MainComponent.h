#pragma once

#include <JuceHeader.h>

using namespace juce;



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
                       public juce::FilenameComponentListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

    
    
    virtual void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged) override
    {
        lorisManager->set("freqdrift", "50.0");
        
        //auto freq = 146.8323839587038; // cello
        auto freq = 329.6275569128699; // (piano 2);
        
        lorisManager->analyse({{fc.getCurrentFile(), freq}}, nullptr);
        
        auto gainList = lorisManager->getSnapshot(fc.getCurrentFile(), 0.5, "bandwidth")[0];
        
        DBG(JSON::toString(gainList));
        
        //auto pitchEnv = lorisManager->createEnvelope(fc.getCurrentFile(), Identifier("rootFrequency"), 0);
        
        Identifier id("gain");
        
        var pitchEnv;
        
        for(int i = 0; i < 8; i++)
        {
            if(!pitchEnv.isBuffer())
                pitchEnv = lorisManager->createEnvelope(fc.getCurrentFile(), id, i)[0];
            else
                *pitchEnv.getBuffer() += *lorisManager->createEnvelope(fc.getCurrentFile(), id, i)[0].getBuffer();
            
            auto e = lorisManager->setEnvelope(pitchEnv, id);
            
            harmonics.add(e);
        }
        
        
        
        
        auto bl = lorisManager->synthesise(fc.getCurrentFile());
        
        thumbnail.setBuffer(bl[0], bl[1]);
        
        resized();
    }
    
    
    
private:
    
    Array<Path> harmonics;
    
    Colour pathColours[8];
    
    //==============================================================================
    // Your private member variables go here...

    LorisManager::Ptr lorisManager;
    
    String error;
    
    Rectangle<float> errorArea;
    juce::FilenameComponent fc;
    
    juce::TextEditor log;
    
    Rectangle<int> previewArea;
    
    hise::HiseAudioThumbnail thumbnail;

    hise::GlobalHiseLookAndFeel laf;
    
    juce::TextEditor commandInput;
    juce::TextButton compileButton;
    juce::TextButton resetButton;
    juce::ComboBox commandOptions;
    juce::ComboBox timeDomain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
