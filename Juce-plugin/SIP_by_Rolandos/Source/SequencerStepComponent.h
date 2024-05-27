/*
  ==============================================================================

    SequencerStepComponent.h
    Created: 27 May 2024 12:15:35am
    Author:  ricca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class SequencerStepComponent  : public juce::Component
{
public:
    SequencerStepComponent(SIPquencerAudioProcessor& processor, int row, int step);
    ~SequencerStepComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;


private:
    SIPquencerAudioProcessor& audioProcessor;
    int row;
    int step;

    juce::Rectangle<float> stepRectangle;

    juce::Colour getColorForStep(int row, int step) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerStepComponent)
};
