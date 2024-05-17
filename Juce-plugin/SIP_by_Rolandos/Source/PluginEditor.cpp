/*
    ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

    ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIP_by_RolandosAudioProcessorEditor::SIP_by_RolandosAudioProcessorEditor(SIP_by_RolandosAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    attackSliders{
        AttachedSlider(p, param::PID::GainWet),
        AttachedSlider(p, param::PID::Frequency),
        AttachedSlider(p, param::PID::GainWet),
    },
    decaySliders{
        AttachedSlider(p, param::PID::GainWet),
        AttachedSlider(p, param::PID::Frequency),
        AttachedSlider(p, param::PID::GainWet),
    }   
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(750, 600);
    startTimerHz(40);
    
    //buttonGrid = std::make_unique<KeyButtonGrid>(3, 3);
    // Initialize the step sequencer grid with all steps off
    stepSequencerGrid.resize(numRows);
    for (int row = 0; row < numRows; ++row)
    {
        stepSequencerGrid[row].resize(numSteps);
        for (int step = 0; step < numSteps; ++step)
        {
            stepSequencerGrid[row][step] = false;
        }
    }

    // Calculate the size and position of the grid rectangle
    int sequencerWidth = getWidth();
    int sequencerHeight = getHeight() * 0.5;
    int seqX = 0;
    int seqY = 0;
    sequencerRect.setBounds(seqX, seqY, sequencerWidth, sequencerHeight);
    seqControlsRect.setBounds(sequencerRect.getX(), sequencerRect.getY(), 130, sequencerRect.getHeight());
    stepGridRect.setBounds(sequencerRect.getX() + 130, sequencerRect.getY(), sequencerRect.getWidth() - 130, sequencerRect.getHeight());

    // Calculate the size and position of the grid rectangle
    int phoneWidth = getWidth();
    int phoneHeight = getHeight() * 0.5;
    int phnX = 0;
    int phnY = getHeight() * 0.5;
    phoneRect.setBounds(phnX, phnY, phoneWidth, phoneHeight);
}


SIP_by_RolandosAudioProcessorEditor::~SIP_by_RolandosAudioProcessorEditor()
{
    // Stop the timer when the editor is destroyed
    stopTimer();
}

    //==============================================================================
void SIP_by_RolandosAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
       
    drawSequencerRows(g, sequencerRect);
    drawButtonGrid(g, phoneRect);
    drawSeqControls(g, seqControlsRect);

}

    
void SIP_by_RolandosAudioProcessorEditor::drawSeqControls(juce::Graphics& g, const juce::Rectangle<int>& bounds) {
    //g.setColour(juce::Colours::red); // Change to the desired color

    //// Fill the rectangle with the specified color
    //g.fillRect(bounds);
    int padding = 2;
    int rowHeight = (bounds.getHeight() - (numRows - 1) * padding) / numRows;

    //// Draw ComboBoxes
    for (int row = 0; row < numRows; ++row)
    {
        int comboBoxY = bounds.getY() + row * (rowHeight + padding);
        juce::Rectangle<int> seqControlBounds(bounds.getX(), comboBoxY, bounds.getWidth(), rowHeight);
        attackSliders[row].slider.setBounds(seqControlBounds.getX(), seqControlBounds.getY(), seqControlBounds.getWidth() * 0.75, seqControlBounds.getHeight() * 0.4);
        decaySliders[row].slider.setBounds(seqControlBounds.getX(), seqControlBounds.getY() + seqControlBounds.getHeight() * 0.6, seqControlBounds.getWidth() * 0.75, seqControlBounds.getHeight() * 0.4);
        // Set the color for the border
        g.setColour(juce::Colours::black);

        // Draw the border for each comboBoxBounds rectangle
        g.drawRect(seqControlBounds, 1); // Border width is set to 1 pixel

        sequenceControls.add(seqControlBounds);
    }

}

void SIP_by_RolandosAudioProcessorEditor::drawSequencerRows(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{

    //// Draw ComboBoxes
    //for (int row = 0; row < numRows; ++row)
    //{
    //    int comboBoxY = bounds.getY() + row * (rowHeight + padding);
    //    juce::Rectangle<int> comboBoxBounds(comboBoxRect.getX(), comboBoxY, comboBoxRect.getWidth() * 0.75, rowHeight * 0.4);
    //    sequenceControls.add(comboBoxBounds);
    //}

    // Draw the step grid
    drawStepGrid(g, stepGridRect);

}

//void SIP_by_RolandosAudioProcessorEditor::drawStepRectangles(juce::Graphics& g, const juce::Rectangle<int>& bounds, int row)
//{
//    // Calculate the width and height of each step rectangle
//    int padding = 2; // Padding between rectangles
//    int stepWidth = (bounds.getWidth() - (numSteps - 1) * padding) / numSteps;
//    int stepHeight = bounds.getHeight();
//
//    for (int step = 0; step < numSteps; ++step)
//    {
//        // Calculate the position and size of the step rectangle
//        int stepX = bounds.getX() + step * (stepWidth + padding);
//        juce::Rectangle<int> stepRectangle(stepX, bounds.getY(), stepWidth, stepHeight);
//        DBG("step: " << step);
//        DBG("audioProcessor.getCurrentStep(): " << audioProcessor.getCurrentStep());
//
//        // Determine the color of the step rectangle based on whether the step is on or off
//        if(audioProcessor.getCurrentStep() == step)
//			g.setColour(juce::Colours::red);
//		else if (stepSequencerGrid[row][step])
//            g.setColour(juce::Colours::green);
//        else
//            g.setColour(juce::Colours::grey);
//
//        // Draw the filled rectangle (step)
//        g.fillRect(stepRectangle);
//
//        // Draw the border for each step rectangle
//        g.setColour(juce::Colours::black);
//        g.drawRect(stepRectangle, 1); // Border width is set to 1 pixel
//    }
//}

void SIP_by_RolandosAudioProcessorEditor::drawStepGrid(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    // Calculate the height of each row
    int rowHeight = bounds.getHeight() / numRows;

    for (int row = 0; row < numRows; ++row)
    {
        // Calculate the bounds for the current row
        juce::Rectangle<int> rowBounds(bounds.getX(), bounds.getY() + row * rowHeight, bounds.getWidth(), rowHeight);

        // Draw the row
        drawRow(g, rowBounds, row);
    }
}

void SIP_by_RolandosAudioProcessorEditor::drawRow(juce::Graphics& g, const juce::Rectangle<int>& bounds, int row)
{
    if (audioProcessor.getSelectedSequence() == row)
    {
        g.setColour(juce::Colours::green);
        g.fillRect(bounds);
    }

    // Calculate the width of each step
    int stepWidth = bounds.getWidth() / numSteps;

    for (int step = 0; step < numSteps; ++step)
    {
        // Calculate the bounds for the current step
        juce::Rectangle<int> stepBounds(bounds.getX() + step * stepWidth, bounds.getY(), stepWidth, bounds.getHeight());
                
        // Determine the color of the step rectangle based on whether the step is currently playing
        // and whether it's the selected row

       juce::Colour stepcolor = getColorForStep(row,step);
        g.setColour(stepcolor);

        // Draw the filled rectangle (step)
        g.fillRect(stepBounds);

        // Draw the border for each step rectangle
        g.setColour(juce::Colours::black);
        g.drawRect(stepBounds, 1); // Border width is set to 1 pixel
    }
}

void SIP_by_RolandosAudioProcessorEditor::drawButtonGrid(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    int buttonSize = 50; // Define the size of each button

    // Iterate over buttons in the map and draw them
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;

        // Calculate the position of the button based on row and column
        int xPos = bounds.getX() + button.col * (buttonSize + 10); // Adjust as needed
        int yPos = bounds.getY() + button.row * (buttonSize + 10); // Adjust as needed

        // Define the bounds for the button
        juce::Rectangle<int> buttonBounds(xPos, yPos, buttonSize, buttonSize);

        // Draw the button
        drawButton(g, buttonBounds, button);
    }
}


void SIP_by_RolandosAudioProcessorEditor::drawButton(juce::Graphics& g, const juce::Rectangle<int>& bounds, const KeyButton button)
{
    // Set color based on button state
    if (button.pressed)
        g.setColour(juce::Colours::green);
    else if(button.playing)
        g.setColour(juce::Colours::yellow);
    else 
        g.setColour(juce::Colours::blue);

    // Draw button background
    g.fillRect(bounds);

    // Draw button border
    g.setColour(juce::Colours::purple);
    g.drawRect(bounds, 1); // Border width is set to 1 pixel

    // Draw button label (for demonstration purposes)
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(16.0f));
    juce::String midiKeyText = juce::String(button.midiKey);
    g.drawFittedText(midiKeyText, bounds, juce::Justification::centred, 1);
}



juce::Colour SIP_by_RolandosAudioProcessorEditor::getRowColorForRow(int row) const
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

juce::Colour SIP_by_RolandosAudioProcessorEditor::getColorForStep(int row,int step) const
{
   
    // You can define your own color scheme for different rows
   if(audioProcessor.getCurrentStep() == step && audioProcessor.getSequenceStep(row,step)>0)
		return juce::Colours::yellow;
	else if (audioProcessor.getSequenceStep(row, step) > 0)
		return juce::Colours::darkorange;
	else if (audioProcessor.getCurrentStep() == step)
		return juce::Colours::darkred;
    else if (audioProcessor.getSelectedStep() == step)
       return juce::Colours::darkcyan;
   else
        return juce::Colours::grey;
}





void SIP_by_RolandosAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
        // Calculate the size of the grid rectangle
  
    resizeSeqControls();
}

void SIP_by_RolandosAudioProcessorEditor::resizeSeqControls() {
    //auto w = static_cast<float>(seqControlsRect.getWidth() * 0.3);
    //auto h = static_cast<float>(seqControlsRect.getHeight() * 0.8);

    //auto x = 0.0f;
       
    //auto y = 0.0f;
    int padding = 2;
    for (auto i = 0; i < attackSliders.size(); ++i) {
        int x = sequenceControls.getRectangle(i).getX();
        int y = sequenceControls.getRectangle(i).getY();
        int w = sequenceControls.getRectangle(i).getWidth();
        int h = sequenceControls.getRectangle(i).getHeight();

            
    }


}

void SIP_by_RolandosAudioProcessorEditor::timerCallback()
{

    // Repaint the GUI
    repaint();
}
