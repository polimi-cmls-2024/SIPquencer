/*
  ==============================================================================

    SequencerGridComponent.cpp
    Created: 26 May 2024 1:50:47pm
    Author:  ricca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SequencerGridComponent.h"
#include <memory>
#include "SequencerStepComponent.h"
#include "PluginProcessor.h"
//==============================================================================
SequencerGridComponent::SequencerGridComponent(SIPquencerAudioProcessor& processor,int rows, int steps)
    : audioProcessor(processor), numRows(rows), numSteps(steps)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setVisible(true);

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numSteps; j++) {
            SequencerStepComponent * step = new SequencerStepComponent(audioProcessor,i,j );
            stepsMap[i][j] = step;
            addAndMakeVisible(stepsMap[i][j]);
        }
    }
    
}

SequencerGridComponent::~SequencerGridComponent()
{

}

void SequencerGridComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

  


    paintQuarters(g);
    paintSteps(g);
}

void SequencerGridComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    container = getLocalBounds().toFloat().reduced(7.0f,7.0f);

    setSequencerBounds(container);
   
}



void SequencerGridComponent::setSequencerBounds(juce::Rectangle<float> container)
{
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        juce::Rectangle<float> quarterRect;
        int quarterWidth = container.getWidth() / 4;
        quarterRect.setBounds(container.getX() + quarter * quarterWidth, container.getY(), container.getWidth() / 4, container.getHeight());
        quarterRect.removeFromLeft(3);
        quarterRect.removeFromRight(3);
        setQuarterBounds(quarterRect, quarter);
        quarterColums[quarter] = quarterRect;
    }
}

void SequencerGridComponent::setQuarterBounds(juce::Rectangle<float> container,int quarter) {

    for (int row = 0; row < numRows; row++) {
        juce::Rectangle<float> quarterRowRect;
        int quarterRowHeight = container.getHeight() / numRows;
        quarterRowRect.setBounds(container.getX(), container.getY() + row * quarterRowHeight, container.getWidth(), quarterRowHeight);
        setQuarterStepsBounds(quarterRowRect.withTrimmedLeft(2).withTrimmedRight(2), row, quarter);
        quarterRowRects[row][quarter] = quarterRowRect;
    };

}

void SequencerGridComponent::setQuarterStepsBounds(juce::Rectangle<float> container, int row, int quarter) {
    for (int step = 0; step < numSteps / 4; step++) {
        juce::Rectangle<float> stepContainerRect;
        int stepContinerWidth = container.getWidth() / 4;
        int stepContainerHeight = container.getHeight();
        int stepContainerX = container.getX() + step * stepContinerWidth;
        int stepContainerY = container.getY();
        stepContainerRect.setBounds(stepContainerX, stepContainerY, stepContinerWidth, stepContainerHeight);
        setStepBounds(stepContainerRect,row, quarter*4 +step);
    }
}

void SequencerGridComponent::setStepBounds(juce::Rectangle<float> container, int row,int step) {
    juce::Rectangle<float> stepRect;
    int stepWidth = container.getWidth() - 5;
    int stepHeight = stepWidth;
    int stepX = container.getCentreX() - stepWidth*0.5;
    int stepY = container.getCentreY() - stepHeight * 0.5;
    stepRect.setBounds(stepX, stepY, stepWidth, stepHeight);
    stepRects[row][step] = stepRect;
    stepsMap[row][step]->setBounds(stepRects[row][step].toNearestInt());
}

void  SequencerGridComponent::paintQuarters(juce::Graphics& g) {
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        juce::DropShadow shdw;
        shdw.colour = juce::Colours::darkgrey;
        shdw.radius = 10;
        shdw.drawForRectangle(g, quarterColums[quarter].getSmallestIntegerContainer());


        g.setColour(juce::Colour(178, 162, 133)); // Example color
        g.fillRoundedRectangle(quarterColums[quarter], 10.f);
        g.setColour(juce::Colours::darkgrey);
        // Draw the border around the rectangle
        g.drawRoundedRectangle(quarterColums[quarter], 10.f, 3);
    }
}

void SequencerGridComponent::paintSteps(juce::Graphics& g) {
    for (int step = 0; step < numSteps; step++) {
        for (int row = 0; row < numRows; row++) {
            juce::DropShadow innerBottomRowShdw;
            innerBottomRowShdw.colour = getColorForStep(row, step);
            innerBottomRowShdw.radius = 10;
            innerBottomRowShdw.drawForRectangle(g, stepRects[row][step].toNearestInt());

            g.setColour(getColorForStep(row, step));
            g.fillRoundedRectangle(stepRects[row][step], 10.f);
            g.setColour(getColorForStep(row, step).darker(0.2));
            g.drawRoundedRectangle(stepRects[row][step], 10.f, 2);

            int note = audioProcessor.getSequenceStep(row, step);
            if (note > 0) {
                juce::String label = juce::MidiMessage::getMidiNoteName(note, true, true, 3);
                g.drawFittedText(label, stepRects[row][step].getSmallestIntegerContainer(), juce::Justification::centred, 1);
            }
           
          /*  int note = audioProcessor.getSequenceStep(row, step);
            if (note > 0) {
                juce::String label = juce::MidiMessage::getMidiNoteName(note, true, true, 3);
                gr.drawFittedText(label, stepRects[row][step].getSmallestIntegerContainer(), juce::Justification::centred, 1);
            }*/
        }
    }
}

juce::Colour SequencerGridComponent::getRowColorForRow(int row) const
{
    // You can define your own color scheme for different rows
    switch (row)
    {
    case 0:
        return juce::Colours::lightblue;
    case 1:
        return juce::Colours::lightgreen;
    case 2:
        return juce::Colours::lightyellow;
    default:
        return juce::Colours::white;
    }
}

juce::Colour SequencerGridComponent::getColorForStep(int row, int step) const
{
    // You can define your own color scheme for different rows
    if (audioProcessor.getCurrentStep() == step && audioProcessor.getSequenceStep(row, step) > 0)
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

void SequencerGridComponent::repaintGrid() {
        repaint();
    
}