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

//struct AttachedSlider
//{
//    using Slider = juce::Slider;
//    using Attachment = juce::SliderParameterAttachment;
//    using Component = juce::Component;
//    AttachedSlider(SIP_by_RolandosAudioProcessor& processor, param::PID pID) :
//        slider(),
//        attachment(*processor.params[static_cast<int>(pID)], slider, nullptr)
//    {
//        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
//    }
//
//    Slider slider;
//    Attachment attachment;
//};

//==============================================================================
/**
*/
class SIP_by_RolandosAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                             private juce::Timer

{
   

public:

    SIP_by_RolandosAudioProcessorEditor (SIP_by_RolandosAudioProcessor&);
    ~SIP_by_RolandosAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;



    std::map<int, juce::Slider*> attackSliders;
    std::map<int, juce::Slider*> releaseSliders;
    std::map<int, juce::Slider*> cutoffSliders;

    juce::Slider reverbSlider;
    juce::Slider delaySlider;





    std::map<int, juce::ComboBox*> instrSelectors; // OwnedArray for managing ComboBox pointers
    juce::ComboBox effectSelector;

    SIP_by_RolandosAudioProcessor& audioProcessor;

private:
// This reference is provided as a quick way for your editor to
// access the processor object that created it. 
    std::vector<std::vector<bool>> stepSequencerGrid; // Repr   esents the step sequencer grid

    const int numRows = 3;
    const int numSteps = 16;

    // Timer callback
    void timerCallback() override;

    juce::Rectangle<int> mainContainerRect;
    juce::Rectangle<int> topRowRect;
    juce::Rectangle<int> bottomRowRect;
    juce::Rectangle<int> effectsRect;
    juce::Rectangle<int> keysContainer;
    juce::Rectangle<int> controlsColumn;
    juce::Rectangle<int> sequencerColumn;
    std::map<int, juce::Rectangle<int>> quarterColums;
    std::map<int, juce::Rectangle<int>> quarterRows;
    std::map<int, std::map<int,juce::Rectangle<int>>> quarterRowRects;
    std::map<int, std::map<int, juce::Rectangle<int>>> stepRects;

    std::map<int, juce::Rectangle<int>> controlRows;
    std::map<int, juce::Rectangle<int>> selectorContainers;
    std::map<int, juce::Rectangle<int>> attackContainers;
    std::map<int, juce::Rectangle<int>> releaseContainers;
    std::map<int, juce::Rectangle<int>> cutoffContainers;

    juce::Rectangle<int> reverbContainer;
    juce::Rectangle<int> delayContainer;



    std::map<int, juce::Rectangle<int>> buttonGridRects;
    std::map<int, juce::Rectangle<int>> sideButtonsRects;




    juce::Rectangle<int> sequencerRect; // Rectangle to contain the step sequencer grid
    juce::Rectangle<int> seqControlsRect;  // Rectangle for ComboBoxes
    juce::Rectangle<int> stepGridRect;  // Rectangle for the step grid
    juce::Rectangle<int> phoneRect; // Rectangle to contain the step sequencer grid
    juce::RectangleList<int> sequenceControls;



    juce::Colour getRowColorForRow(int row) const;
    juce::Colour getColorForStep(int row,int step) const;
    juce::Colour getButtonColor(const KeyButton button) const;
    juce::String getButtonLabel(const KeyButton button) const;
   
    void drawButton(juce::Graphics& g, const juce::Rectangle<int>& bounds, KeyButton button);
  


    void setControlsBounds(juce::Rectangle<int> container);
    void setControlBounds(juce::Rectangle<int> container,int row);
    void setSequencerBounds(juce::Rectangle<int> container);
    void setQuarterBounds(juce::Rectangle<int> container,int quarter);
    void setQuarterStepsBounds(juce::Rectangle<int> container, int row, int quarter);
    void setStepBounds(juce::Rectangle<int> container, int row, int step);
    void setEffectsBounds(juce::Rectangle<int> container);
    void setKeyboardBounds(juce::Rectangle<int> container);
    void setButtonGridBounds(const juce::Rectangle<int>& gridBounds);
    void setSideButtonBounds(const juce::Rectangle<int>& sideButtonsContainer);
    void setEffectKnobsBounds(juce::Rectangle<int> container);


    void selectInstrument(int selectorIndex);
    void selectControl();

    //*****************************************************************************
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_by_RolandosAudioProcessorEditor)
};
