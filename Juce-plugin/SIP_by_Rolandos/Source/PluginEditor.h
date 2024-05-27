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
#include "KeyPadComponent.h"
#include "SequenceControlsComponent.h"
#include "SequencerGridComponent.h"

namespace juce {
    class CustomLNF : public juce::LookAndFeel_V4 {
    public:
        void drawRotarySlider(Graphics&, int x, int y, int width, int height,
            float sliderPosProportional, float rotaryStartAngle,
            float rotaryEndAngle, Slider&);

        static const Font getCustomFont() {
            //static auto typeface = Typeface::createSystemTypefaceFor(BinaryData::Harabara_Mais_otf, BinaryData::Harabara_Mais_otfSize);
            static auto typeface = Typeface::createSystemTypefaceFor(BinaryData::DREPHONIC_ttf, BinaryData::DREPHONIC_ttfSize);
            //static auto typeface = Typeface::createSystemTypefaceFor(BinaryData::Tweety_otf, BinaryData::Tweety_otfSize); // might be too simple
            //static auto typeface = Typeface::createSystemTypefaceFor(BinaryData::coolvetica_rg_otf, BinaryData::coolvetica_rg_otfSize);
            return Font(typeface);
        }

        Typeface::Ptr getTypefaceForFont(const Font& f) override {
            return getCustomFont().getTypeface();
        }
    };
}



//==============================================================================
/**
*/
class SIPquencerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                             private juce::Timer,
                                             juce::AudioProcessorParameter::Listener
{
   

public:

    SIPquencerAudioProcessorEditor (SIPquencerAudioProcessor&);
    ~SIPquencerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    //void paintQuarters(juce::Graphics&);
    //void paintSteps(juce::Graphics&);
    //void paintButtonGrid(juce::Graphics&);
    //void paintSideButtons(juce::Graphics&);
    void parameterValueChanged(int parameterIndex, float newValue) override;    
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}    
	

    juce::Slider* reverbSlider;
    juce::Slider* delaySlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;

    
    juce::Image sip_logo = juce::ImageCache::getFromMemory(BinaryData::SIPquencer_logo_png, BinaryData::SIPquencer_logo_pngSize);
    juce::Rectangle<int> effectSelectorContainer;
    juce::Rectangle<float> knobsContainer;


    juce::ComboBox* effectSelector;

    SIPquencerAudioProcessor& audioProcessor;

private:
// This reference is provided as a quick way for your editor to
// access the processor object that created it. 

    KeyPadComponent keypad;
    SequencerGridComponent sequencerGrid;
    std::array<SequenceControlsComponent*, 3> sequenceControls;


    juce::Rectangle<float> bottomRow;
    juce::Rectangle<float> topRow;
    juce::Rectangle<float> effectsRect;
    juce::Rectangle<float> keyPadContainer;
    juce::Rectangle<float> controlsColumn;
    juce::Rectangle<int> reverbContainer;
    juce::Rectangle<int> delayContainer;

    juce::Rectangle<float> sequencerColumn;

    
    std::map<int,juce::Rectangle<float>> controlRows;



    const int numRows = 3;
    const int numSteps = 16;

            
    // Timer callback
    void timerCallback() override;



    juce::Label bpmLabel;


    void setSequencerBounds(juce::Rectangle<float> container);

    void setControlsBounds(juce::Rectangle<float> container);
 
    void setEffectsBounds(juce::Rectangle<float> container);

    void setEffectKnobsBounds(juce::Rectangle<float> container);


    //void selectInstrument(int selectorIndex);
    void selectControl();

    juce::CustomLNF myCustomLNF;


    //*****************************************************************************
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIPquencerAudioProcessorEditor)
};
