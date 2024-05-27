/*
  ==============================================================================

    SequencerGridComponent.h
    Created: 26 May 2024 1:50:47pm
    Author:  ricca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h" 
#include "SequencerStepComponent.h"
#include <memory>


//==============================================================================
/*
*/
class SequencerGridComponent  : public juce::Component
{
public:
    SequencerGridComponent(SIPquencerAudioProcessor& processor,int rows, int steps);
    ~SequencerGridComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;



    void paintQuarters(juce::Graphics&);
    void paintSteps(juce::Graphics&);



    juce::Colour getRowColorForRow(int row) const;
    juce::Colour getColorForStep(int row, int step) const;
    void setSequencerBounds(juce::Rectangle<float> container);
    void setQuarterBounds(juce::Rectangle<float> container, int quarter);
    void setQuarterStepsBounds(juce::Rectangle<float> container, int row, int quarter);
    void setStepBounds(juce::Rectangle<float> container, int row, int step);
    void repaintGrid();

private:

    //std::vector<std::vector<bool>> stepSequencerGrid; // Represents the step sequencer grid

    SIPquencerAudioProcessor& audioProcessor;


    int numRows;
    int numSteps;


    juce::Rectangle<float> container;

    std::map<int, std::map<int, SequencerStepComponent*>> stepsMap;

    std::map<int, juce::Rectangle<float>> quarterColums;
    std::map<int, juce::Rectangle<float>> quarterRows;
    std::map<int, std::map<int, juce::Rectangle<float>>> quarterRowRects;
    std::map<int, std::map<int, juce::Rectangle<float>>> stepRects;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerGridComponent)
};

    