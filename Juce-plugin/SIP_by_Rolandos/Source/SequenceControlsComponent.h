/*
  ==============================================================================

    SequenceControlsComponent.h
    Created: 26 May 2024 1:51:57pm
    Author:  ricca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"    

//==============================================================================
/*
*/
class SequenceControlsComponent  : public juce::Component
{
public:
    SequenceControlsComponent(SIPquencerAudioProcessor& processor,int index);
    ~SequenceControlsComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;


    void setControlBounds(juce::Rectangle<float> container);
    void selectInstrument();

private:
    SIPquencerAudioProcessor& audioProcessor;
    int sequenceIndex;

    juce::Slider* attackSlider;
    juce::Slider* releaseSlider;
    juce::Slider* cutoffSlider;
    juce::ComboBox* instrSelector; // OwnedArray for managing ComboBox pointers


    std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr < juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;


    juce::Rectangle<float> controlsContainer;
    juce::Rectangle<float> selectorContainer;
    juce::Rectangle<float> attackContainer;
    juce::Rectangle<float> releaseContainer;
    juce::Rectangle<float> cutoffContainer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequenceControlsComponent)
};
