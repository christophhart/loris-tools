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
    void resized() override;

    virtual void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged) override
    {
        lorisManager->analyse({{fc.getCurrentFile(), 440.0}}, nullptr);
        
        
        auto bl = lorisManager->synthesise(fc.getCurrentFile());
        
        
        
        
        
        thumbnail.setBuffer(bl[0], bl[1]);
        
        repaint();
    }
    
    
    
private:
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
