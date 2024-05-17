/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Params.h"
#include <array>

struct AttachedSlider
{
    using Slider = juce::Slider;
    using Attachment = juce::SliderParameterAttachment;
    using Component = juce::Component;
    AttachedSlider(SIP_by_RolandosAudioProcessor& processor, param::PID pID) :
        slider(),
        attachment(*processor.params[static_cast<int>(pID)], slider, nullptr)
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    }

    Slider slider;
    Attachment attachment;
};

//==============================================================================
/**
*/
class SIP_by_RolandosAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                             private juce::Timer

{
    using AttachedSliderArray = std::array<AttachedSlider, 3>;
   

public:

    SIP_by_RolandosAudioProcessorEditor (SIP_by_RolandosAudioProcessor&);
    ~SIP_by_RolandosAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    AttachedSliderArray attackSliders;
    AttachedSliderArray decaySliders;

    SIP_by_RolandosAudioProcessor& audioProcessor;

private:
// This reference is provided as a quick way for your editor to
// access the processor object that created it. 
    std::vector<std::vector<bool>> stepSequencerGrid; // Repr   esents the step sequencer grid

    const int numRows = 3;
    const int numSteps = 16;

    // Timer callback
    void timerCallback() override;

    juce::Rectangle<int> sequencerRect; // Rectangle to contain the step sequencer grid
    juce::Rectangle<int> seqControlsRect;  // Rectangle for ComboBoxes
    juce::Rectangle<int> stepGridRect;  // Rectangle for the step grid
    juce::Rectangle<int> phoneRect; // Rectangle to contain the step sequencer grid
    juce::RectangleList<int> sequenceControls;
    

    juce::Colour getRowColorForRow(int row) const;
    juce::Colour getColorForStep(int row,int step) const;

   
    void drawButtonGrid(juce::Graphics&     g, const juce::Rectangle<int>& bounds);
    void drawButton(juce::Graphics& g, const juce::Rectangle<int>& bounds, KeyButton button);
    //void drawStepRectangles(juce::Graphics& g, const juce::Rectangle<int>& bounds, int row);

    void drawSequencerRows(juce::Graphics& g, const juce::Rectangle<int>& bounds);

    void drawStepGrid(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    void drawSeqControls(juce::Graphics& g, const juce::Rectangle<int>& bounds);
    void drawRow(juce::Graphics& g, const juce::Rectangle<int>& bounds, int row);
    void resizeSeqControls();
    //*****************************************************************************
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_by_RolandosAudioProcessorEditor)
};
