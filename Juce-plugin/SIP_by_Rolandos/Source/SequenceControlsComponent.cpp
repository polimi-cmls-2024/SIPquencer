/*
  ==============================================================================

    SequenceControlsComponent.cpp
    Created: 26 May 2024 1:51:57pm
    Author:  ricca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SequenceControlsComponent.h"
#include "PluginProcessor.h"

//==============================================================================
SequenceControlsComponent::SequenceControlsComponent(SIPquencerAudioProcessor& processor,int index)
	: audioProcessor(processor), sequenceIndex(index),attackSlider(),releaseSlider(),cutoffSlider(),instrSelector()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    attackSlider = new juce::Slider();
    attackSlider->setComponentID("Attack"+std::to_string(sequenceIndex));
    attackSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    attackSlider ->setTextBoxStyle(juce::Slider::NoTextBox, true, 30, 20);
    attackSlider->setRange(0, 1);
    attackSlider->setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(attackSlider); 


    releaseSlider = new juce::Slider();
    releaseSlider->setComponentID("Release " + std::to_string(sequenceIndex));
    releaseSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    releaseSlider->setTextBoxStyle(juce::Slider::NoTextBox, true, 30, 20);
    releaseSlider->setRange(0, 1);
    releaseSlider->setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(releaseSlider); 
 

    cutoffSlider = new juce::Slider();
    cutoffSlider->setComponentID("Cutoff " + std::to_string(sequenceIndex));
    cutoffSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    cutoffSlider->setTextBoxStyle(juce::Slider::NoTextBox, true, 30, 20);
    cutoffSlider->setRange(20, 20000);
    cutoffSlider->setNumDecimalPlacesToDisplay(2);
    addAndMakeVisible(cutoffSlider); 

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "attack" + std::to_string(sequenceIndex), *attackSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "release" + std::to_string(sequenceIndex), *releaseSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "cutoff" + std::to_string(sequenceIndex), *cutoffSlider);

    instrSelector = new juce::ComboBox();
    instrSelector->addItem("Instr. 1", 1);
    instrSelector->addItem("Instr. 2", 2);
    instrSelector->addItem("Instr. 3", 3);

    // Capture 'row' by value to ensure each lambda captures its own value of 'row'
    instrSelector->onChange = [this] { selectInstrument(); };
    instrSelector->setSelectedId(1);
    addAndMakeVisible(instrSelector);
    // Add ComboBox pointer to the map


}

SequenceControlsComponent::~SequenceControlsComponent()
{
}

void SequenceControlsComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    juce::DropShadow shdw;
    shdw.colour = juce::Colours::darkgrey;
    shdw.radius = 5;
    shdw.drawForRectangle(g, controlsContainer.toNearestInt());

    
    if (sequenceIndex == audioProcessor.getSelectedSequence()+1) {
            g.setColour(juce::Colour(114, 144, 159));
    }
    else {
        g.setColour(juce::Colour(178, 162, 133)); // Example color
    }
    g.fillRoundedRectangle(controlsContainer, 5.f);
    g.setColour(juce::Colours::darkgrey); // Example color
    g.drawRoundedRectangle(controlsContainer, 5.f, 2);
    
}

void SequenceControlsComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    setControlBounds(getLocalBounds().toFloat().reduced(4.0f,4.0f));
}



void SequenceControlsComponent::setControlBounds(juce::Rectangle<float> container) {
    controlsContainer = container;
    float margin = 2;

    juce::Rectangle<float> selectorContainer;
    float selectorContainerWidth = container.getWidth();
    float selectorContainerHeight = container.getHeight() * 0.3;
    float selectorContainerX = container.getX();
    float selectorContainerY = container.getY();
    selectorContainer.setBounds(selectorContainerX, selectorContainerY, selectorContainerWidth, selectorContainerHeight);
    selectorContainer.reduce(margin, margin);
    instrSelector->setBounds(selectorContainer.getSmallestIntegerContainer());

    juce::Rectangle<float> attackContainer;
    float attackContainerWidth = container.getWidth() / 3;
    float attackContainerHeight = container.getHeight() * 0.7;
    float attackContainerX = container.getX();
    float attackContainerY = container.getY() + selectorContainerHeight;
    attackContainer.setBounds(attackContainerX, attackContainerY, attackContainerWidth, attackContainerHeight);
    attackSlider->setBounds(attackContainer.getSmallestIntegerContainer());


    juce::Rectangle<float> releaseContainer;
    float releaseContainerWidth = container.getWidth() / 3;
    float releaseContainerHeight = container.getHeight() * 0.7;
    float releaseContainerX = container.getX() + attackContainerWidth;
    float releaseContainerY = container.getY() + selectorContainerHeight;
    releaseContainer.setBounds(releaseContainerX, releaseContainerY, releaseContainerWidth, releaseContainerHeight);
    releaseSlider->setBounds(releaseContainer.getSmallestIntegerContainer());



    juce::Rectangle<float> cutoffContainer;
    float cutoffContainerWidth = container.getWidth() / 3;
    float cutoffContainerHeight = container.getHeight() * 0.7;
    float cutoffContainerX = releaseContainerX + releaseContainerWidth;
    float cutoffContainerY = container.getY() + selectorContainerHeight;
    cutoffContainer.setBounds(cutoffContainerX, cutoffContainerY, cutoffContainerWidth, cutoffContainerHeight);
    cutoffSlider->setBounds(cutoffContainer.getSmallestIntegerContainer());


}


void SequenceControlsComponent::selectInstrument() {
    DBG("selectorIndex: ", selectorIndex);
    audioProcessor.setSelectedInstrument(sequenceIndex, instrSelector->getSelectedId());

}
