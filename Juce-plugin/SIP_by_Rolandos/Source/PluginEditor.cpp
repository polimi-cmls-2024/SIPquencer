/*
    ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

    ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SequenceControlsComponent.h"
#include "KeyPadComponent.h"
#include "SequencerGridComponent.h"


namespace juce {
    void CustomLNF::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
    {
        auto radius = jmin(width / 2.25, height / 2.25) - 4.5;
        auto centreX = x + width * 0.4975f;
        auto centreY = y + height * 0.44f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();


        // Knob shadow
        g.setColour(Colours::darkgrey.withAlpha(0.7f));
        g.fillEllipse(rx, ry + 3, rw, rw);

        // Knob fill
        g.setColour(Colour(192, 194, 192));
        g.fillEllipse(rx, ry, rw, rw);


        // Knob outline
        g.setColour(Colour(120, 120, 120));
        g.drawEllipse(rx, ry, rw, rw, 0.5f);

        // Knob pointer
        Path p;
        auto pointerLength = radius * 0.5f;
        auto pointerThickness = width * 0.05f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(Colours::whitesmoke);
        g.fillPath(p);

        // Value
        /*
        g.setColour(Colours::white);
        g.setFont(radius * 0.6f);
        g.drawSingleLineText(String(slider.getValue()), centreX, height * 0.5f,
            Justification::centred);
        */

        // Label
        g.setColour(Colours::whitesmoke);
        g.setFont(radius * 0.75f);

    /*    g.drawSingleLineText(slider.getComponentID(), centreX,
            height, Justification::centredRight);*/


    }

}
//==============================================================================
SIPquencerAudioProcessorEditor::SIPquencerAudioProcessorEditor(SIPquencerAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    reverbSlider(),
    delaySlider(),
    effectSelector(),
    keypad(p),
    sequencerGrid(p,3,16),
    sequenceControls()
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(40);
    juce::LookAndFeel::setDefaultLookAndFeel(&myCustomLNF);

    addAndMakeVisible(sequencerGrid);
    addAndMakeVisible(keypad);

    for (int i = 0; i < numRows; i++) {
        SequenceControlsComponent * control = new SequenceControlsComponent(audioProcessor, i+1);
        sequenceControls[i] =control;

    }


    reverbSlider = new juce::Slider();
    reverbSlider->setComponentID("Reverb");
    reverbSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    reverbSlider->setTextBoxStyle(juce::Slider::NoTextBox, true, 40, 15);
    reverbSlider->setRange(0, 1);
    reverbSlider->setNumDecimalPlacesToDisplay(2);
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "reverb", *reverbSlider);

    addAndMakeVisible(reverbSlider);


    delaySlider = new juce::Slider();
    delaySlider->setComponentID("Delay");
    delaySlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    delaySlider->setTextBoxStyle(juce::Slider::NoTextBox, true, 40, 15);
    delaySlider->setRange(0, 1);
    delaySlider->setNumDecimalPlacesToDisplay(2);
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                audioProcessor.apvts, "delay", *delaySlider);

    addAndMakeVisible(delaySlider);

    effectSelector = new juce::ComboBox();
    effectSelector->addSectionHeading("Phone control");
    effectSelector->addSeparator();
    effectSelector->addItem("Attack", 1);
    effectSelector->addItem("Release", 2);
    effectSelector->addItem("Cutoff", 3);
    effectSelector->addItem("Reverb", 4);
    effectSelector->addItem("Delay",5);
    //effectSelector.setColour(1,juce::Colours::azure);

    // Capture 'row' by value to ensure each lambda captures its own value of 'row'
    effectSelector->onChange = [this]() {
        selectControl();
        };
    effectSelector->setSelectedId(1);
    audioProcessor.setSelectedControl(effectSelector->getText().toLowerCase());
    addAndMakeVisible(effectSelector);

    const auto& params = audioProcessor.getParameters();

    for (auto param : params) {
        param->addListener(this);
    }

    for (int i = 0; i < numRows; i++) {
        addAndMakeVisible(sequenceControls[i]);
    }
    setSize(800, 600);

}


SIPquencerAudioProcessorEditor::~SIPquencerAudioProcessorEditor()
{
    // Stop the timer when the editor is destroyed
    stopTimer();
    // Delete ComboBox pointers
}

    //==============================================================================
void SIPquencerAudioProcessorEditor::paint(juce::Graphics& g)
{
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.setColour(juce::Colour(24, 39, 61)); // Example color
        g.fillRect(getLocalBounds());

        //paintMainContainer()
        //g.setColour(juce::Colours::purple); // Example color
        //g.fillRoundedRectangle(topRowRect,20.f);



        g.setColour(juce::Colour(234, 216, 184)); // Example color
        g.fillRoundedRectangle(bottomRow, 30.f);
        g.setColour(juce::Colour(178, 162, 133)); // Example color
        g.drawRoundedRectangle(bottomRow, 30.f, 3);
        juce::DropShadow bottomRowShdw;
        bottomRowShdw.colour = juce::Colours::darkgrey;
        bottomRowShdw.radius = 30;

        bottomRowShdw.drawForRectangle(g, bottomRow.toNearestInt()
            .withTrimmedBottom(5)
            .withTrimmedLeft(10)
            .withTrimmedRight(10)
            .withTrimmedTop(3));

        g.setColour(juce::Colour(234, 216, 184)); // Example color
        g.fillRoundedRectangle(bottomRow.toFloat()
            .withTrimmedBottom(5)
            .withTrimmedLeft(10)
            .withTrimmedRight(10)
            .withTrimmedTop(3), 30.f);


        g.setColour(juce::Colour(234, 216, 184)); // Example color
        g.fillRoundedRectangle(topRow.toFloat(), 30.f);
        g.setColour(juce::Colour(178, 162, 133)); // Example color
        g.drawRoundedRectangle(topRow.toFloat(), 30.f, 3);


        juce::DropShadow innerBottomRowShdw;
        innerBottomRowShdw.colour = juce::Colours::darkgrey;
        innerBottomRowShdw.radius = 8;
        innerBottomRowShdw.drawForRectangle(g, effectsRect.getSmallestIntegerContainer().getUnion(keyPadContainer.getSmallestIntegerContainer()));

    g.setColour(juce::Colour(114,144,159)); // Example color
    g.fillRoundedRectangle(effectsRect.getUnion(keyPadContainer.toFloat()),8.f);
    g.setColour(juce::Colours::darkgrey); // Example color
    g.drawRoundedRectangle(effectsRect.getUnion(keyPadContainer.toFloat()), 8.f, 3);
    g.drawImageWithin(sip_logo, effectsRect.getX() + effectsRect.getWidth() * 0.15, effectsRect.getY() + effectsRect.getHeight() * 0.1, sip_logo.getWidth() / 7, sip_logo.getHeight() / 7,
        juce::RectanglePlacement::stretchToFit);
    //juce::DropShadow logoShdw;
    //logoShdw.colour = juce::Colours::darkgrey;
    //logoShdw.radius = 2;
    //logoShdw.drawForImage(g, sip_logo);





    //g.setColour(juce::Colours::azure);
    //g.drawRoundedRectangle(keysContainer, 8.f, 3);
    //g.setColour(juce::Colours::azure);
    //g.drawRoundedRectangle(sideButtonBounds, 8.f, 3);

    g.setColour(juce::Colour(234, 216, 184)); // Example color
    g.fillRect(controlsColumn);

    /*g.setColour(juce::Colours::pink);
    g.fillRect(sequencerColumn);*/



    for (int row = 0; row < numRows; row++) {
        //if (row == audioProcessor.getSelectedSequence()) {
        //    g.setColour(juce::Colour(114, 144, 159));
        //}
        //else {
        //    g.setColour(juce::Colour(178, 162, 133)); // Example color
        //}
        //g.fillRoundedRectangle(controlRows[row], 5.f);
        //g.setColour(juce::Colours::darkgrey); // Example color
        //g.drawRoundedRectangle(controlRows[row], 5.f, 2);
        //sequenceControls[row]->paint(g);

        //g.setColour(juce::Colours::cadetblue); // Example color

        //g.fillRect(selectorContainers[row]);
        //      g.setColour(juce::Colours::coral); // Example color
              //g.fillRoundedRectangle(attackContainers[row],5.f);
        //      g.setColour(juce::Colours::lightskyblue); // Example color
              //g.fillRoundedRectangle(releaseContainers[row],5.f);
        //sequenceControls[row]->paint(g);

    }



}


void SIPquencerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    // Calculate the size of the grid rectangle
    // Calculate the size and position of the grid rectangle
    juce::Rectangle<float> mainContainer = getLocalBounds().toFloat();
 
    int topRowWidth = mainContainer.getWidth() - 10;
    int topRowHeight = mainContainer.getHeight() * 0.45 -10;
    int topRowX = mainContainer.getX() + 5;
    int topRowY = mainContainer.getY() + 5;
    topRow.setBounds(topRowX, topRowY, topRowWidth, topRowHeight);
    setSequencerBounds(topRow.reduced(20));


    int bottomRowWidth = mainContainer.getWidth() -10;
    int bottomRowHeight = mainContainer.getHeight() * 0.55 -10;
    int bottomRowX = mainContainer.getX() +5;
    int bottomRowY = topRowY + topRowHeight + 5;
    bottomRow.setBounds(bottomRowX, bottomRowY, bottomRowWidth, bottomRowHeight);
    juce::Rectangle<float> innerBottomRowRect = bottomRow.withTrimmedLeft(90)
                                        .withTrimmedRight(90)
                                        .withTrimmedTop(20)
                                        .withTrimmedBottom(40);
    keyPadContainer = innerBottomRowRect;
    keypad.setBounds(keyPadContainer.getSmallestIntegerContainer());

  

    int effectsWidth = innerBottomRowRect.getWidth() *0.25;
    int effectsHeight = innerBottomRowRect.getHeight();
    int effectsX = innerBottomRowRect.getX();
    int effectsY = innerBottomRowRect.getY();
    effectsRect.setBounds(effectsX, effectsY, effectsWidth, effectsHeight);
    setEffectsBounds(effectsRect);

    //juce::Rectangle<float> keyPadContainer;
    //int keysContainerWidth = innerBottomRowRect.getWidth()- effectsWidth;
    //int keysContainerHeight = innerBottomRowRect.getHeight();
    //int keysContainerX = innerBottomRowRect.getX() + effectsWidth;
    //int keysContainerY = innerBottomRowRect.getY();
    //keyPadContainer.setBounds(keysContainerX, keysContainerY, keysContainerWidth, keysContainerHeight);
    //keypad.setBounds(keyPadContainer.getSmallestIntegerContainer());

}

void SIPquencerAudioProcessorEditor::setSequencerBounds(juce::Rectangle<float> container)
{
    int controlsColumnWidth = container.getWidth() *0.25;
    int controlsColumnHeight = container.getHeight();
    int controlsColumnX = container.getX();
    int controlsColumnY = container.getY();
    controlsColumn.setBounds(controlsColumnX, controlsColumnY, controlsColumnWidth, controlsColumnHeight);
    setControlsBounds(controlsColumn);
    

    int sequencerColumnWidth = container.getWidth() - controlsColumn.getWidth();
    int sequencerColumnHeight = container.getHeight();
    int sequencerColumnX = controlsColumn.getX() + controlsColumn.getWidth();
    int sequencerColumnY = container.getY();
    sequencerColumn.setBounds(sequencerColumnX, sequencerColumnY, sequencerColumnWidth, sequencerColumnHeight);
    sequencerGrid.setBounds(sequencerColumn.getSmallestIntegerContainer());
}


void SIPquencerAudioProcessorEditor::setControlsBounds(juce::Rectangle<float> container) {
    int margin = 3;
    for (int row = 0; row < numRows; ++row) {
        juce::Rectangle<float> rowRect;
		int rowWidth = container.getWidth();
		int rowHeight = container.getHeight() / numRows;
		int rowX = container.getX();
		int rowY = container.getY() + row * rowHeight;
		rowRect.setBounds(rowX, rowY, rowWidth, rowHeight);
        rowRect.reduce(margin,margin);
        controlRows[row] = rowRect;
	    sequenceControls[row]->setBounds(controlRows[row].toNearestInt());

    }
} 



void SIPquencerAudioProcessorEditor::setEffectsBounds(juce::Rectangle<float> container) {

    // Define the margin to reduce the size of the inner rectangle
    float margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<float> innerContainer = container.reduced(margin);
    //juce::Rectangle<int> effectSelectorContainer;
    effectSelectorContainer.setBounds(innerContainer.getX() + innerContainer.getWidth() * 0.2, innerContainer.getY() + innerContainer.getHeight() * 0.7, innerContainer.getWidth(), innerContainer.getHeight() * 0.2);


    //juce::Rectangle<float> knobsContainer;
    knobsContainer.setBounds(innerContainer.getX() + innerContainer.getWidth() * 0.2, innerContainer.getY() + innerContainer.getHeight() * 0.2, innerContainer.getWidth(), innerContainer.getHeight() * 0.8);
    

    effectSelectorContainer.reduce(margin, margin);
    effectSelector->setBounds(effectSelectorContainer);
    knobsContainer.reduce(margin, margin);
    setEffectKnobsBounds(knobsContainer);

}

void SIPquencerAudioProcessorEditor::setEffectKnobsBounds(juce::Rectangle<float> container) {
    // Define the margin to reduce the size of the inner rectangle
    float margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<float> innerContainer = container.reduced(margin);
    reverbContainer.setBounds(innerContainer.getX(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);
    delayContainer.setBounds(innerContainer.getX() + reverbContainer.getWidth(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);
    reverbContainer.reduce(margin, margin);
    delayContainer.reduce(margin, margin);
    reverbSlider->setBounds(reverbContainer);
    delaySlider->setBounds(delayContainer);

}

void SIPquencerAudioProcessorEditor::timerCallback()
{

    // Repaint the GUI
    ////repaint();
    sequencerGrid.repaintGrid();
    keypad.repaint();
    sequenceControls[audioProcessor.getSelectedSequence()]->repaint();
}



void SIPquencerAudioProcessorEditor::selectControl() {
    DBG("selected control: ", effectSelector.getSelectedId());
    audioProcessor.setSelectedControl(effectSelector->getText().toLowerCase());

}


void SIPquencerAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue) {
	DBG("parameterValueChanged: "+std::to_string(parameterIndex));
	DBG("newValue: "+ std::to_string(newValue));
    DBG("id: " + audioProcessor.getParameterID(parameterIndex));
    audioProcessor.sendControlChange(parameterIndex, newValue);
	//audioProcessor.setParameter(parameterIndex, newValue);
}