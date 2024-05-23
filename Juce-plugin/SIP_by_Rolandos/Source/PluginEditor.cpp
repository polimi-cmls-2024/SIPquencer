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
    releaseSliders()

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
        attackSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        attackSlider->setTextBoxStyle(juce::Slider::TextBoxBelow,true, 40, 15);
        attackSlider->setRange(0,1);
        attackSlider->setNumDecimalPlacesToDisplay(2);
        attackSliders[row] = attackSlider;

        juce::Slider* releaseSlider = new juce::Slider();
        releaseSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        releaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
        releaseSlider->setRange(0, 1);
        releaseSlider->setNumDecimalPlacesToDisplay(2);

        releaseSliders[row] = releaseSlider;

        juce::Slider* cutoffSlider = new juce::Slider();
        cutoffSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        cutoffSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
        cutoffSlider->setRange(20, 20000);
        cutoffSlider->setNumDecimalPlacesToDisplay(2);
        cutoffSliders[row] = cutoffSlider;

    }

    reverbSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    reverbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
    reverbSlider.setRange(0, 1);
    reverbSlider.setNumDecimalPlacesToDisplay(2);


    delaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
    delaySlider.setRange(0, 1);
    delaySlider.setNumDecimalPlacesToDisplay(2);


    effectSelector.addItem("Attack", 1);
    effectSelector.addItem("Release", 2);
    effectSelector.addItem("Cutoff", 3);
    effectSelector.addItem("Reverb", 4);
    effectSelector.addItem("Delay",5);


    // Capture 'row' by value to ensure each lambda captures its own value of 'row'
    effectSelector.onChange = [this]() {
        selectControl();
        };
    effectSelector.setSelectedId(1);


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
		g.fillRect(releaseContainers[row]);
	}
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        g.setColour(juce::Colours::azure); // Example color
        g.fillRect(quarterColums[quarter]);
        g.setColour(juce::Colours::black);
        // Draw the border around the rectangle
        g.drawRect(quarterColums[quarter], 3);
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

    int keysContainerWidth = bottomRowRect.getWidth()- effectsWidth;
    int keysContainerHeight = bottomRowRect.getHeight();
    int keysContainerX = effectsRect.getX() + effectsWidth;
    int keysContainerY = bottomRowRect.getY();
    keysContainer.setBounds(keysContainerX, keysContainerY, keysContainerWidth, keysContainerHeight);
    setKeyboardBounds(keysContainer);

}

void SIP_by_RolandosAudioProcessorEditor::setSequencerBounds(juce::Rectangle<int> container)
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
    int margin = 5;
    for (int row = 0; row < numRows; ++row) {
        juce::Rectangle<int> rowRect;
		int rowWidth = container.getWidth();
		int rowHeight = container.getHeight() / numRows;
		int rowX = container.getX();
		int rowY = container.getY() + row * rowHeight;
		rowRect.setBounds(rowX, rowY, rowWidth, rowHeight);
        rowRect.reduce(margin,margin);
		setControlBounds(rowRect,row);
        controlRows[row] = rowRect;
    }
}

void SIP_by_RolandosAudioProcessorEditor::setControlBounds(juce::Rectangle<int> container,int row) {
    int margin = 2;
    juce::Rectangle<int> selectorContainer;
    int selectorContainerWidth = container.getWidth();
    int selectorContainerHeight = container.getHeight() * 0.3;
    int selectorContainerX = container.getX();
    int selectorContainerY = container.getY();
    selectorContainer.setBounds(selectorContainerX, selectorContainerY, selectorContainerWidth, selectorContainerHeight);
    selectorContainer.reduce(margin, margin);
    selectorContainers[row] = selectorContainer;
    if (instrSelectors.find(row) != instrSelectors.end()) {
        instrSelectors[row]->setBounds(selectorContainer); // Corrected
        addAndMakeVisible(instrSelectors[row]); // Add Slider to the editor

    }

    juce::Rectangle<int> attackContainer;
    int attackContainerWidth = container.getWidth()/3;
    int attackContainerHeight = container.getHeight() * 0.7;
    int attackContainerX = container.getX();
    int attackContainerY = container.getY() + selectorContainerHeight;
    attackContainer.setBounds(attackContainerX, attackContainerY, attackContainerWidth, attackContainerHeight);
    attackContainers[row] = attackContainer;
    if (attackSliders.find(row) != attackSliders.end()) {
        attackSliders[row]->setBounds(attackContainer); // Corrected
        addAndMakeVisible(attackSliders[row]); // Add Slider to the editor

    }


    juce::Rectangle<int> releaseContainer;
    int releaseContainerWidth = container.getWidth() / 3;
    int releaseContainerHeight = container.getHeight() * 0.7;
    int releaseContainerX = container.getX() + attackContainerWidth;
    int releaseContainerY = container.getY() + selectorContainerHeight;
    releaseContainer.setBounds(releaseContainerX, releaseContainerY, releaseContainerWidth, releaseContainerHeight);
    releaseContainers[row] = releaseContainer;
    if (releaseSliders.find(row) != releaseSliders.end()) {
        releaseSliders[row]->setBounds(releaseContainer); // Corrected
        addAndMakeVisible(releaseSliders[row]); // Add Slider to the editor
    }

    juce::Rectangle<int> cutoffContainer;
    int cutoffContainerWidth = container.getWidth() / 3;
    int cutoffContainerHeight = container.getHeight() * 0.7;
    int cutoffContainerX = releaseContainerX + releaseContainerWidth;
    int cutoffContainerY = container.getY() + selectorContainerHeight;
    cutoffContainer.setBounds(cutoffContainerX, cutoffContainerY, cutoffContainerWidth, cutoffContainerHeight);
    cutoffContainers[row] = cutoffContainer;
    if (cutoffSliders.find(row) != cutoffSliders.end()) {   
        cutoffSliders[row]->setBounds(cutoffContainer); // Corrected
        addAndMakeVisible(cutoffSliders[row]); // Add Slider to the editor
    }

}



void SIP_by_RolandosAudioProcessorEditor::setEffectsBounds(juce::Rectangle<int> container) {

    // Define the margin to reduce the size of the inner rectangle
    int margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<int> innerContainer = container.reduced(margin);
    juce::Rectangle<int> effectSelectorContainer;
    effectSelectorContainer.setBounds(innerContainer.getX(), innerContainer.getY(), innerContainer.getWidth(), innerContainer.getHeight() * 0.2);


    juce::Rectangle<int> knobsContainer;
    knobsContainer.setBounds(innerContainer.getX(), innerContainer.getY() + effectSelectorContainer.getHeight(), innerContainer.getWidth(), innerContainer.getHeight() * 0.8);

    effectSelectorContainer.reduce(margin, margin);
    effectSelector.setBounds(effectSelectorContainer);
    addAndMakeVisible(effectSelector);
    knobsContainer.reduce(margin, margin);
    setEffectKnobsBounds(knobsContainer);

}

void SIP_by_RolandosAudioProcessorEditor::setEffectKnobsBounds(juce::Rectangle<int> container) {
    // Define the margin to reduce the size of the inner rectangle
    int margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<int> innerContainer = container.reduced(margin);
    reverbContainer.setBounds(innerContainer.getX(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);


    delayContainer.setBounds(innerContainer.getX() + reverbContainer.getWidth(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);
    reverbContainer.reduce(margin, margin);
    delayContainer.reduce(margin, margin);
    reverbSlider.setBounds(reverbContainer);
    addAndMakeVisible(reverbSlider);
    delaySlider.setBounds(delayContainer);
    addAndMakeVisible(delaySlider);

}

void SIP_by_RolandosAudioProcessorEditor::setKeyboardBounds(juce::Rectangle<int> container) {
  
    juce::Rectangle<int> gridBounds;
    int gridBoundX = container.getX();
    int gridBoundY = container.getY();
    int gridBoundWidth = container.getWidth() * 0.75;
    int gridBoundHeight = container.getHeight();
    gridBounds.setBounds(gridBoundX, gridBoundY, gridBoundWidth, gridBoundHeight);
    juce::Rectangle<int> sideButtonBounds;
    int sideButtonBoundsX = container.getX() + gridBoundWidth;
    int sideButtonBoundsY = container.getY();
    int sideButtonBoundsWidth = container.getWidth() - gridBoundWidth;
    int sideButtonBoundsHeight = container.getHeight() * 0.25;
    sideButtonBounds.setBounds(sideButtonBoundsX, sideButtonBoundsY, sideButtonBoundsWidth, sideButtonBoundsHeight);

    // Draw the button grid in the gridBounds rectangle
    setButtonGridBounds(gridBounds);

    // Draw the side buttons in the sideButtonBounds rectangle
    setSideButtonBounds(sideButtonBounds);
}

void SIP_by_RolandosAudioProcessorEditor::setButtonGridBounds(const juce::Rectangle<int>& gridBounds)
{
    int buttonSize = 50; // Define the size of each button
    int spacing = 10; // Define the spacing between buttons

    // Determine the number of rows and columns
    int numRows = 0;
    int numCols = 0;
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;
        if (button.row + 1 > numRows) numRows = button.row + 1;
        if (button.col + 1 > numCols) numCols = button.col + 1;
    }

    // Calculate the total size of the button grid
    int totalGridWidth = numCols * buttonSize + (numCols - 1) * spacing;
    int totalGridHeight = numRows * buttonSize + (numRows - 1) * spacing;

    // Calculate the offsets to center the grid
    int xOffset = (gridBounds.getWidth() - totalGridWidth) / 2;
    int yOffset = (gridBounds.getHeight() - totalGridHeight) / 2;

    // Iterate over buttons in the map and set their bounds
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;

        // Calculate the position of the button based on row and column
        int xPos = gridBounds.getX() + xOffset + button.col * (buttonSize + spacing);
        int yPos = gridBounds.getY() + yOffset + button.row * (buttonSize + spacing);

        // Define the bounds for the button
        juce::Rectangle<int> buttonBounds(xPos, yPos, buttonSize, buttonSize);

        // Store the bounds in the buttonGridRects map
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

void SIP_by_RolandosAudioProcessorEditor::selectControl() {
    DBG("selected control: ", effectSelector.getSelectedId());
}