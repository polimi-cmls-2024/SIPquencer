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
                                             private juce::Timer,
                                             juce::AudioProcessorParameter::Listener
{
   

public:

    SIP_by_RolandosAudioProcessorEditor (SIP_by_RolandosAudioProcessor&);
    ~SIP_by_RolandosAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void paintView();
    void paintQuarters(juce::Graphics&);
    void paintSteps(juce::Graphics&);
    void paintButtonGrid(juce::Graphics&);
    void paintSideButtons(juce::Graphics&);
    void parameterValueChanged(int parameterIndex, float newValue) override;    
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}    
	

    std::map<int, juce::Slider*> attackSliders;
    std::map<int, juce::Slider*> releaseSliders;
    std::map<int, juce::Slider*> cutoffSliders;

    juce::Slider reverbSlider;
    juce::Slider delaySlider;


    // Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attackAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> releaseAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> cutoffAttachments;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;

    
    juce::Image sip_logo = juce::ImageCache::getFromMemory(BinaryData::sipquencer_small_resized_png, BinaryData::sipquencer_small_resized_pngSize);
    juce::Rectangle<int> effectSelectorContainer;
    juce::Rectangle<float> knobsContainer;


    std::map<int, juce::ComboBox*> instrSelectors; // OwnedArray for managing ComboBox pointers
    juce::ComboBox effectSelector;

    SIP_by_RolandosAudioProcessor& audioProcessor;

private:
// This reference is provided as a quick way for your editor to
// access the processor object that created it. 

   
    juce::Image view;
    std::vector<std::vector<bool>> stepSequencerGrid; // Repr   esents the step sequencer grid

    const int numRows = 3;
    const int numSteps = 16;
            
    // Timer callback
    void timerCallback() override;

    juce::Rectangle<float> mainContainerRect;
    juce::Rectangle<float> topRowRect;
    juce::Rectangle<float> bottomRowRect;
    juce::Rectangle<float> innerBottomRowRect;

   
    juce::Rectangle<float> effectsRect;
    juce::Rectangle<float> keysContainer;
    juce::Rectangle<float> controlsColumn;
    juce::Rectangle<float> sequencerColumn;
    std::map<int, juce::Rectangle<float>> quarterColums;
    std::map<int, juce::Rectangle<float>> quarterRows;
    std::map<int, std::map<int,juce::Rectangle<float>>> quarterRowRects;
    std::map<int, std::map<int, juce::Rectangle<float>>> stepRects;

    std::map<int, juce::Rectangle<float>> controlRows;
    std::map<int, juce::Rectangle<float>> selectorContainers;
    std::map<int, juce::Rectangle<float>> attackContainers;
    std::map<int, juce::Rectangle<float>> releaseContainers;
    std::map<int, juce::Rectangle<float>> cutoffContainers;

    juce::Rectangle<int> reverbContainer;
    juce::Rectangle<int> delayContainer;

    juce::Rectangle<float> buttonGridBounds;
    juce::Rectangle<float> sideButtonBounds;
    juce::Label bpmLabel;


    std::map<int, juce::Rectangle<float>> buttonGridRects;
    std::map<int, juce::Rectangle<float>> sideButtonsRects;




    juce::Rectangle<float> sequencerRect; // Rectangle to contain the step sequencer grid
    juce::Rectangle<float> seqControlsRect;  // Rectangle for ComboBoxes
    juce::Rectangle<float> stepGridRect;  // Rectangle for the step grid
    juce::Rectangle<float> phoneRect; // Rectangle to contain the step sequencer grid
    juce::RectangleList<float> sequenceControls;



    juce::Colour getRowColorForRow(int row) const;
    juce::Colour getColorForStep(int row,int step) const;
    juce::Colour getButtonColor(const KeyButton button) const;
    juce::Colour getSideButtonColor(const KeyButton button) const;
    juce::String getButtonLabel(const KeyButton button) const;

    

   
    void drawButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, KeyButton button);
    void drawSideButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, KeyButton button);


    void setControlsBounds(juce::Rectangle<float> container);
    void setControlBounds(juce::Rectangle<float> container,int row);
    void setSequencerBounds(juce::Rectangle<float> container);
    void setQuarterBounds(juce::Rectangle<float> container,int quarter);
    void setQuarterStepsBounds(juce::Rectangle<float> container, int row, int quarter);
    void setStepBounds(juce::Rectangle<float> container, int row, int step);
    void setEffectsBounds(juce::Rectangle<float> container);
    void setKeyboardBounds(juce::Rectangle<float> container);
    void setButtonGridBounds(const juce::Rectangle<float>& gridBounds);
    void setSideButtonBounds(const juce::Rectangle<float>& sideButtonsContainer);
    void setEffectKnobsBounds(juce::Rectangle<float> container);


    void selectInstrument(int selectorIndex);
    void selectControl();
    //*****************************************************************************
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_by_RolandosAudioProcessorEditor)
};
