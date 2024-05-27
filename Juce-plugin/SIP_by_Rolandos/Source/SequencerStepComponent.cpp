/*
  ==============================================================================

    SequencerStepComponent.cpp
    Created: 27 May 2024 12:15:35am
    Author:  ricca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SequencerStepComponent.h"

//==============================================================================
SequencerStepComponent::SequencerStepComponent(SIPquencerAudioProcessor& processor, int row, int step)
    : audioProcessor(processor), row(row), step(step)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setVisible(true);
}

SequencerStepComponent::~SequencerStepComponent()
{
}

void SequencerStepComponent::paint(juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    


   /* juce::DropShadow innerBottomRowShdw;
    innerBottomRowShdw.colour = getColorForStep(row, step);
    innerBottomRowShdw.radius = 10;
    innerBottomRowShdw.drawForRectangle(g, stepRectangle.toNearestInt());*/

  
  /*  g.setColour(getColorForStep(row, step));
    g.fillRoundedRectangle(stepRectangle, 10.f);
    g.setColour(getColorForStep(row, step).darker(0.2));
    g.drawRoundedRectangle(stepRectangle, 10.f, 2);*/

   /* int note = audioProcessor.getSequenceStep(row, step);
    if (note > 0) {
        juce::String label = juce::MidiMessage::getMidiNoteName(note, true, true, 3);
        g.drawFittedText(label, stepRectangle.getSmallestIntegerContainer(), juce::Justification::centred, 1);
    }*/
}

 juce::Colour SequencerStepComponent::getColorForStep(int row, int step) const
{
    // You can define your own color scheme for different rows
   if(audioProcessor.getCurrentStep() == step && audioProcessor.getSequenceStep(row,step)>0)
		return juce::Colours::yellow;
	else if (audioProcessor.getSequenceStep(row, step) > 0)
		return juce::Colours::darkorange;
	else if (audioProcessor.getCurrentStep() == step && audioProcessor.tempoRunning())
		return juce::Colour(255, 66, 82);
    else if (audioProcessor.getSelectedStep() == step)
       return juce::Colours::darkcyan;
   else
        return juce::Colours::grey;
}
void SequencerStepComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    stepRectangle = getLocalBounds().toFloat();
}
