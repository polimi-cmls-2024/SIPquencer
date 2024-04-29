/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SIP_sequencerAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Slider::Listener
{
public:
    SIP_sequencerAudioProcessorEditor (SIP_sequencerAudioProcessor&);
    ~SIP_sequencerAudioProcessorEditor();

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void setLabelText(float freqHz);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SIP_sequencerAudioProcessor& processor;
    
    juce::Label freqLabel;
    juce::DatagramSocket ds;
    juce::OSCSender sender;
    
    juce::Slider reverb;
    juce::Label reverbLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_sequencerAudioProcessorEditor)
    
    /*void showConnectionErrorMessage (const String& messageText)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Connection error",
                                          messageText,
                                          "OK");
    }*/
    void sliderValueChanged (juce::Slider* slider) ;
};
