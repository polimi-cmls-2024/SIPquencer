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
    attackSliders(),
    decaySliders()

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
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

        juce::ComboBox* selector = new juce::ComboBox(); // Allocate memory for ComboBox
        selector->addItem("Instr. 1", 1);
        selector->addItem("Instr. 2", 2);
        selector->addItem("Instr. 3", 3);

        // Capture 'row' by value to ensure each lambda captures its own value of 'row'
        selector->onChange = [this, row] { selectInstrument(row); };
        selector->setSelectedId(1);

        // Add ComboBox pointer to the map
        instrSelectors[row] = selector;

        juce::Slider* attackSlider = new juce::Slider();
        attackSliders[row] = attackSlider;

        juce::Slider* decaySlider = new juce::Slider();
        decaySliders[row] = decaySlider;


    }
        setSize(750, 600);

}


SIP_by_RolandosAudioProcessorEditor::~SIP_by_RolandosAudioProcessorEditor()
{
    // Stop the timer when the editor is destroyed
    stopTimer();
    // Delete ComboBox pointers
    for (auto& pair : instrSelectors)
    {
        delete pair.second; // Delete each ComboBox pointer
    }
}

    //==============================================================================
void SIP_by_RolandosAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::blue); // Example color
    g.fillRect(mainContainerRect);

    g.setColour(juce::Colours::purple); // Example color
    g.fillRect(topRowRect);

    g.setColour(juce::Colours::pink); // Example color
    g.fillRect(bottomRowRect);

    g.setColour(juce::Colours::orange); // Example color
    g.fillRect(effectsRect);

    g.setColour(juce::Colours::yellow); // Example color
    g.fillRect(keysContainer);

    g.setColour(juce::Colours::red); // Example color
    g.fillRect(controlsColumn);

    for (int row = 0; row < numRows; row++) {
        g.setColour(juce::Colours::rebeccapurple); // Example color
		g.fillRect(controlRows[row]);
        g.setColour(juce::Colours::cadetblue); // Example color

		g.fillRect(selectorContainers[row]);
        g.setColour(juce::Colours::coral); // Example color
		g.fillRect(attackContainers[row]);
        g.setColour(juce::Colours::lightskyblue); // Example color
		g.fillRect(decayContainers[row]);
	}
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        g.setColour(juce::Colours::azure); // Example color
        g.fillRect(quarterColums[quarter]);
	}
    for (int row = 0; row < numRows; row++) {
		for (int step = 0; step < numSteps; step++) {
			g.setColour(getColorForStep(row,step));
			g.fillRect(stepRects[row][step]);
		}
	}
	for (const auto& entry : buttonGridRects)
	{
		const juce::Rectangle<int>& buttonRect = entry.second;
		const KeyButton& button = audioProcessor.keyButtonGrid.at(entry.first);
		drawButton(g, buttonRect, button);
	}

	for (const auto& entry : sideButtonsRects)
	{
		const juce::Rectangle<int>& buttonRect = entry.second;
		const KeyButton& button = audioProcessor.sideButtons.at(entry.first);
		drawButton(g, buttonRect, button);
	}
	
}



void SIP_by_RolandosAudioProcessorEditor::drawButton(juce::Graphics& g, const juce::Rectangle<int>& bounds, const KeyButton button)
{
    // Set color based on button state

    juce::Colour buttonColor = getButtonColor(button);
    g.setColour(buttonColor);
   

    // Draw button background
    g.fillRect(bounds);

    // Draw button border
    g.setColour(juce::Colours::purple);
    g.drawRect(bounds, 1); // Border width is set to 1 pixel

    // Draw button label (for demonstration purposes)
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(16.0f));
    juce::String label = getButtonLabel(button);
    g.drawFittedText(label, bounds, juce::Justification::centred, 1);
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

juce::Colour SIP_by_RolandosAudioProcessorEditor::getButtonColor(const KeyButton button) const
{
	// Set color based on button state
    if (button.pressed) {
        return juce::Colours::darkgoldenrod;
    }
    else if (button.playing) {
        return juce::Colours::yellowgreen;
    }
    else {
        switch (audioProcessor.getSequencerState()) {
        case audioProcessor.DEF:
            return juce::Colours::grey;
        case audioProcessor.R:
            return juce::Colours::dodgerblue;
        case audioProcessor.RP:
            return juce::Colours::blanchedalmond;
        case audioProcessor.RRP:
            return juce::Colours::coral;
        }
    }
   
}

juce::String SIP_by_RolandosAudioProcessorEditor::getButtonLabel(const KeyButton button) const
{
    switch (audioProcessor.getSequencerState()) {
    case audioProcessor.DEF:
        return button.defLabel;
    case audioProcessor.R:
        return button.Rlabel;
    case audioProcessor.RP:
        return button.RPlabel;
    case audioProcessor.RRP:
        return button.RRPlabel;
    default:
        DBG("Invalid state");
        return "Invalid state";
    }
}

void SIP_by_RolandosAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
        // Calculate the size of the grid rectangle
    // Calculate the size and position of the grid rectangle
    
    int mainContainerWidth = getWidth();
    int mainContainerHeight = getHeight();
    int mainContainerX = 0;
    int mainContainerY = 0;
    mainContainerRect.setBounds(mainContainerX, mainContainerY, mainContainerWidth, mainContainerHeight);

    int topRowWidth = mainContainerRect.getWidth() - 10;
    int topRowHeight = mainContainerRect.getHeight() * 0.5 -10;
    int topRowX = mainContainerRect.getX() + 5;
    int topRowY = mainContainerRect.getY() + 5;
    topRowRect.setBounds(topRowX, topRowY, topRowWidth, topRowHeight);
    setSequencerBounds(topRowRect);


    int bottomRowWidth = mainContainerRect.getWidth() -10;
    int bottomRowHeight = mainContainerRect.getHeight() * 0.5 -10;
    int bottomRowX = mainContainerRect.getX() +5;
    int bottomRowY = topRowRect.getY() + topRowRect.getHeight() + 5;
    bottomRowRect.setBounds(bottomRowX, bottomRowY, bottomRowWidth, bottomRowHeight);
  

    int effectsWidth = bottomRowRect.getWidth() /3;
    int effectsHeight = bottomRowRect.getHeight();
    int effectsX = bottomRowRect.getX();
    int effectsY = bottomRowRect.getY();
    effectsRect.setBounds(effectsX, effectsY, effectsWidth, effectsHeight);
    setEffectsBounds(effectsRect);

    int keysContainerWidth = bottomRowRect.getWidth()/3;
    int keysContainerHeight = bottomRowRect.getHeight();
    int keysContainerX = effectsRect.getX() + effectsRect.getWidth();
    int keysContainerY = bottomRowRect.getY();
    keysContainer.setBounds(keysContainerX, keysContainerY, keysContainerWidth, keysContainerHeight);
    setKeyboardBounds(keysContainer);

}

void SIP_by_RolandosAudioProcessorEditor::setSequencerBounds(juce::Rectangle<int> container)
{
    int controlsColumnWidth = container.getWidth() *0.15;
    int controlsColumnHeight = container.getHeight();
    int controlsColumnX = container.getX();
    int controlsColumnY = container.getY();
    controlsColumn.setBounds(controlsColumnX, controlsColumnY, controlsColumnWidth, controlsColumnHeight);
    setControlsBounds(controlsColumn);
    

    int sequencerColumnWidth = container.getWidth() - controlsColumn.getWidth();
    int sequencerColumnHeight = container.getHeight();
    int sequencerColumnX = controlsColumn.getX() + controlsColumn.getWidth();
    int sequencerColumnY = container.getY();
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        juce::Rectangle<int> quarterRect;
        int quarterWidth = sequencerColumnWidth / 4;
        int quarterHeight = sequencerColumnHeight;
        int quarterX = sequencerColumnX + quarter * quarterWidth;
        int quarterY = sequencerColumnY;
        quarterRect.setBounds(quarterX, quarterY, quarterWidth, quarterHeight);
        setQuarterBounds(quarterRect,quarter);
        quarterColums[quarter] = quarterRect;
    }
}

void SIP_by_RolandosAudioProcessorEditor::setQuarterBounds(juce::Rectangle<int> container,int quarter) {

    for (int row = 0; row < numRows; row++) {
        juce::Rectangle<int> quarterRowRect;
        int quarterRowWidth = container.getWidth();
        int quarterRowHeight = container.getHeight() / numRows;
        int rowX = container.getX();
        int rowY = container.getY() + row * quarterRowHeight;
        quarterRowRect.setBounds(rowX, rowY, quarterRowWidth, quarterRowHeight);
        setQuarterStepsBounds(quarterRowRect,row,quarter);
        quarterRowRects[row][quarter] = quarterRowRect;
    };

}

void SIP_by_RolandosAudioProcessorEditor::setQuarterStepsBounds(juce::Rectangle<int> container, int row, int quarter) {
    for (int step = 0; step < numSteps / 4; step++) {
        juce::Rectangle<int> stepContainerRect;
        int stepContinerWidth = container.getWidth() / 4;
        int stepContainerHeight = container.getHeight();
        int stepContainerX = container.getX() + step * stepContinerWidth;
        int stepContainerY = container.getY();
        stepContainerRect.setBounds(stepContainerX, stepContainerY, stepContinerWidth, stepContainerHeight);
        setStepBounds(stepContainerRect,row, quarter*4 +step);
    }
}

void SIP_by_RolandosAudioProcessorEditor::setStepBounds(juce::Rectangle<int> container, int row,int step) {
    juce::Rectangle<int> stepRect;
    int stepWidth = container.getWidth() - 10;
    int stepHeight = container.getHeight() - 10;
    int stepX = container.getX() + 5;
    int stepY = container.getY() + 5;
    stepRect.setBounds(stepX, stepY, stepWidth, stepHeight);
    stepRects[row][step] = stepRect;
}

void SIP_by_RolandosAudioProcessorEditor::setControlsBounds(juce::Rectangle<int> container) {

    for (int row = 0; row < numRows; ++row) {
        juce::Rectangle<int> rowRect;
		int rowWidth = container.getWidth();
		int rowHeight = container.getHeight() / numRows;
		int rowX = container.getX();
		int rowY = container.getY() + row * rowHeight;
		rowRect.setBounds(rowX, rowY, rowWidth, rowHeight);
		setControlBounds(rowRect,row);
        controlRows[row] = rowRect;
    }
}

void SIP_by_RolandosAudioProcessorEditor::setControlBounds(juce::Rectangle<int> container,int row) {
    juce::Rectangle<int> selectorContainer;
    int selectorContainerWidth = container.getWidth();
    int selectorContainerHeight = container.getHeight() * 0.5;
    int selectorContainerX = container.getX();
    int selectorContainerY = container.getY();
    selectorContainer.setBounds(selectorContainerX, selectorContainerY, selectorContainerWidth, selectorContainerHeight);
    selectorContainers[row] = selectorContainer;
    if (instrSelectors.find(row) != instrSelectors.end()) {
        instrSelectors[row]->setBounds(selectorContainer); // Corrected
        addAndMakeVisible(instrSelectors[row]); // Add Slider to the editor

    }

    juce::Rectangle<int> attackContainer;
    int attackContainerWidth = container.getWidth() *0.5;
    int attackContainerHeight = container.getHeight() * 0.5;
    int attackContainerX = container.getX();
    int attackContainerY = container.getY() + selectorContainerHeight;
    attackContainer.setBounds(attackContainerX, attackContainerY, attackContainerWidth, attackContainerHeight);
    attackContainers[row] = attackContainer;
    if (attackSliders.find(row) != attackSliders.end()) {
        attackSliders[row]->setBounds(attackContainer); // Corrected
        addAndMakeVisible(attackSliders[row]); // Add Slider to the editor

    }


    juce::Rectangle<int> decayContainer;
    int decayContainerWidth = container.getWidth() * 0.5;
    int decayContainerHeight = container.getHeight() * 0.5;
    int decayContainerX = container.getX() + attackContainerWidth;
    int decayContainerY = container.getY() + selectorContainerHeight;
    decayContainer.setBounds(decayContainerX, decayContainerY, decayContainerWidth, decayContainerHeight);
    decayContainers[row] = decayContainer;
    if (decaySliders.find(row) != decaySliders.end()) {
        decaySliders[row]->setBounds(decayContainer); // Corrected
        addAndMakeVisible(decaySliders[row]); // Add Slider to the editor
    }

}



void SIP_by_RolandosAudioProcessorEditor::setEffectsBounds(juce::Rectangle<int> container) {

}

void SIP_by_RolandosAudioProcessorEditor::setKeyboardBounds(juce::Rectangle<int> container) {
    // Calculate sub-rectangles for the grid and side buttons
    auto gridWidth = container.getWidth() * 0.75;
    auto sideButtonWidth = container.getWidth() * 0.25;

    juce::Rectangle<int> gridBounds = container.withWidth(gridWidth);
    juce::Rectangle<int> sideButtonBounds = container.withLeft(container.getX() + gridWidth).withWidth(sideButtonWidth);

    // Draw the button grid in the gridBounds rectangle
    setButtonGridBounds(gridBounds);

    // Draw the side buttons in the sideButtonBounds rectangle
    setSideButtonBounds(sideButtonBounds);
}

void SIP_by_RolandosAudioProcessorEditor::setButtonGridBounds(const juce::Rectangle<int>& gridBounds)
{
    int buttonSize = 50; // Define the size of each button
    int spacing = 10; // Define the spacing between buttons

    // Iterate over buttons in the map and draw them
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;

        // Calculate the position of the button based on row and column
        int xPos = gridBounds.getX() + button.col * (buttonSize + spacing);
        int yPos = gridBounds.getY() + button.row * (buttonSize + spacing);

        // Define the bounds for the button
        juce::Rectangle<int> buttonBounds(xPos, yPos, buttonSize, buttonSize);
        buttonGridRects[entry.first] = buttonBounds;
        
    }
}

void SIP_by_RolandosAudioProcessorEditor::setSideButtonBounds(const juce::Rectangle<int>& sideButtonsContainer)
{
    int buttonSize = 50; // Define the size of each button
    int spacing = 10; // Define the spacing between buttons
    // Assuming we have two side buttons to draw
    for (const auto& entry : audioProcessor.sideButtons)
    {
        const KeyButton& button = entry.second;
        int xPos = sideButtonsContainer.getX() + button.col * (buttonSize + spacing);
        int yPos = sideButtonsContainer.getY() + button.row * (buttonSize + spacing);

        // Define the bounds for the button
        juce::Rectangle<int> buttonBounds(xPos, yPos, buttonSize, buttonSize);
        sideButtonsRects[entry.first] = buttonBounds;
    }
}


void SIP_by_RolandosAudioProcessorEditor::timerCallback()
{

    // Repaint the GUI
    repaint();
}

void SIP_by_RolandosAudioProcessorEditor::selectInstrument(int selectorIndex) {
    DBG("selectorIndex: ", selectorIndex);
    DBG("selectedInstr: ", instrSelectors.operator[](selectorIndex).getSelectedId());


}