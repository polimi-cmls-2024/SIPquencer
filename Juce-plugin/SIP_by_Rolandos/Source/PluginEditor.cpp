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
    startTimerHz(20);
    
    
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
        attackAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "attack" + std::to_string(row + 1), *attackSlider));

        juce::Slider* releaseSlider = new juce::Slider();
        releaseSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        releaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
        releaseSlider->setRange(0, 1);
        releaseSlider->setNumDecimalPlacesToDisplay(2);
        releaseSliders[row] = releaseSlider;
        releaseAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "release" + std::to_string(row + 1), *releaseSlider));


        juce::Slider* cutoffSlider = new juce::Slider();
        cutoffSlider->setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        cutoffSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
        cutoffSlider->setRange(20, 20000);
        cutoffSlider->setNumDecimalPlacesToDisplay(2);
        cutoffSliders[row] = cutoffSlider;
        cutoffAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "cutoff" + std::to_string(row + 1), *cutoffSlider));
    }



    reverbSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    reverbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
    reverbSlider.setRange(0, 1);
    reverbSlider.setNumDecimalPlacesToDisplay(2);
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "reverb", reverbSlider);



    delaySlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 40, 15);
    delaySlider.setRange(0, 1);
    delaySlider.setNumDecimalPlacesToDisplay(2);
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "delay", delaySlider);


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
    audioProcessor.setSelectedControl(effectSelector.getText().toLowerCase());


    setSize(800, 600);
    view = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);

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
    g.setColour(juce::Colour(24,39,61)); // Example color
    g.fillRect(mainContainerRect);

    //paintMainContainer()
    //g.setColour(juce::Colours::purple); // Example color
    //g.fillRoundedRectangle(topRowRect,20.f);


    g.setColour(juce::Colours::khaki); // Example color
    g.fillRoundedRectangle(bottomRowRect,30.f);
    g.setColour(juce::Colours::darkkhaki); // Example color
    g.drawRoundedRectangle(bottomRowRect, 30.f,3);
    juce::DropShadow bottomRowShdw;
    bottomRowShdw.colour = juce::Colours::darkgrey;
    bottomRowShdw.radius = 30;

    bottomRowShdw.drawForRectangle(g, bottomRowRect.getSmallestIntegerContainer()
        .withTrimmedBottom(5)
        .withTrimmedLeft(10)
        .withTrimmedRight(10)
        .withTrimmedTop(3));

    g.setColour(juce::Colours::khaki); // Example color
    g.fillRoundedRectangle(bottomRowRect
        .withTrimmedBottom(5)
        .withTrimmedLeft(10)
        .withTrimmedRight(10)
        .withTrimmedTop(3), 30.f);

  
    g.setColour(juce::Colours::khaki); // Example color
    g.fillRoundedRectangle(topRowRect, 30.f);
    g.setColour(juce::Colours::darkkhaki); // Example color
    g.drawRoundedRectangle(topRowRect, 30.f, 3);


    juce::DropShadow innerBottomRowShdw;
    innerBottomRowShdw.colour = juce::Colours::darkgrey;
    innerBottomRowShdw.radius = 8;
    innerBottomRowShdw.drawForRectangle(g, effectsRect.getSmallestIntegerContainer().getUnion(keysContainer.getSmallestIntegerContainer()));

    g.setColour(juce::Colour(114,144,159)); // Example color
    g.fillRoundedRectangle(effectsRect.getUnion(keysContainer),8.f);
    g.setColour(juce::Colours::darkgrey); // Example color
    g.drawRoundedRectangle(effectsRect.getUnion(keysContainer), 8.f, 3);

    g.setColour(juce::Colours::azure);
    g.drawRoundedRectangle(keysContainer, 8.f, 3);
    g.setColour(juce::Colours::azure);
    g.drawRoundedRectangle(sideButtonBounds, 8.f, 3);

    g.setColour(juce::Colours::khaki); // Example color
    g.fillRect(controlsColumn);

    for (int row = 0; row < numRows; row++) {
        g.setColour(juce::Colours::darkkhaki); // Example color
		g.fillRoundedRectangle(controlRows[row],5.f);
        g.setColour(juce::Colours::darkgrey); // Example color
        g.drawRoundedRectangle(controlRows[row], 5.f, 2);
        g.setColour(juce::Colours::cadetblue); // Example color

		g.fillRect(selectorContainers[row]);
  //      g.setColour(juce::Colours::coral); // Example color
		//g.fillRoundedRectangle(attackContainers[row],5.f);
  //      g.setColour(juce::Colours::lightskyblue); // Example color
		//g.fillRoundedRectangle(releaseContainers[row],5.f);
	}
   
    paintQuarters(g);
    paintSteps(g);
    paintButtonGrid(g);
    paintSideButtons(g);
	

	
}


void SIP_by_RolandosAudioProcessorEditor::paintView() {
    juce::Graphics gr(view);
    paintQuarters(gr);
    paintSteps(gr);
    paintButtonGrid(gr);
    paintSideButtons(gr);
}

void  SIP_by_RolandosAudioProcessorEditor::paintQuarters(juce::Graphics& gr){
    for (int quarter = 0; quarter < floor(numSteps / 4); quarter++) {
        gr.setColour(juce::Colours::darkgrey); // Example color
        gr.fillRoundedRectangle(quarterColums[quarter], 10.f);
        gr.setColour(juce::Colours::black);
        // Draw the border around the rectangle
        gr.drawRoundedRectangle(quarterColums[quarter], 10.f, 3);
    }
}
void SIP_by_RolandosAudioProcessorEditor::paintSteps(juce::Graphics& gr) {
    for (int step = 0; step < numSteps; step++) {
    for (int row = 0; row < numRows; row++) {
       
            juce::DropShadow innerBottomRowShdw;
            innerBottomRowShdw.colour = getColorForStep(row, step);
            innerBottomRowShdw.radius = 10;
            innerBottomRowShdw.drawForRectangle(gr, stepRects[row][step].getSmallestIntegerContainer());

            gr.setColour(getColorForStep(row, step));
            gr.fillRoundedRectangle(stepRects[row][step], 10.f);
            gr.drawRoundedRectangle(stepRects[row][step], 10.f, 2);
        }
    }
}

void SIP_by_RolandosAudioProcessorEditor::paintButtonGrid(juce::Graphics& gr) {
    for (const auto& entry : buttonGridRects)
    {
        const juce::Rectangle<float>& buttonRect = entry.second;
        const KeyButton& button = audioProcessor.keyButtonGrid.at(entry.first);
        drawButton(gr, buttonRect, button);
    }
}

void SIP_by_RolandosAudioProcessorEditor::paintSideButtons(juce::Graphics& gr) {
    for (const auto& entry : sideButtonsRects)
    {
        const juce::Rectangle<float>& buttonRect = entry.second;
        const KeyButton& button = audioProcessor.sideButtons.at(entry.first);

        drawButton(gr, buttonRect, button);
    }

}

void SIP_by_RolandosAudioProcessorEditor::drawButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, const KeyButton button)
{
    // Set color based on button state

    juce::Colour buttonColor = getButtonColor(button);
    g.setColour(buttonColor);
   

    // Draw button background
    g.fillRoundedRectangle(bounds,5.f);

    //// Draw button border
    //g.setColour(juce::Colours::purple);
    //g.drawRect(bounds, 1); // Border width is set to 1 pixel

    // Draw button label (for demonstration purposes)
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f));
    juce::String label = getButtonLabel(button);
    g.drawFittedText(label, bounds.getSmallestIntegerContainer(), juce::Justification::centred, 1);
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
		return juce::Colour(255, 66, 82);
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
            return juce::Colours::black;
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
    int topRowHeight = mainContainerRect.getHeight() * 0.45 -10;
    int topRowX = mainContainerRect.getX() + 5;
    int topRowY = mainContainerRect.getY() + 5;
    topRowRect.setBounds(topRowX, topRowY, topRowWidth, topRowHeight);
    setSequencerBounds(topRowRect.reduced(20));


    int bottomRowWidth = mainContainerRect.getWidth() -10;
    int bottomRowHeight = mainContainerRect.getHeight() * 0.55 -10;
    int bottomRowX = mainContainerRect.getX() +5;
    int bottomRowY = topRowRect.getY() + topRowRect.getHeight() + 5;
    bottomRowRect.setBounds(bottomRowX, bottomRowY, bottomRowWidth, bottomRowHeight);
    innerBottomRowRect = bottomRowRect.withTrimmedLeft(90)
                                        .withTrimmedRight(90)
                                        .withTrimmedTop(20)
                                        .withTrimmedBottom(40);
  

    int effectsWidth = innerBottomRowRect.getWidth() /3;
    int effectsHeight = innerBottomRowRect.getHeight();
    int effectsX = innerBottomRowRect.getX();
    int effectsY = innerBottomRowRect.getY();
    effectsRect.setBounds(effectsX, effectsY, effectsWidth, effectsHeight);
    setEffectsBounds(effectsRect);

    int keysContainerWidth = innerBottomRowRect.getWidth()- effectsWidth;
    int keysContainerHeight = innerBottomRowRect.getHeight();
    int keysContainerX = innerBottomRowRect.getX() + effectsWidth;
    int keysContainerY = innerBottomRowRect.getY();
    keysContainer.setBounds(keysContainerX, keysContainerY, keysContainerWidth, keysContainerHeight);
    setKeyboardBounds(keysContainer);

}

void SIP_by_RolandosAudioProcessorEditor::setSequencerBounds(juce::Rectangle<float> container)
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
        juce::Rectangle<float> quarterRect;
        int quarterWidth = sequencerColumnWidth / 4;
        int quarterHeight = sequencerColumnHeight;
        int quarterX = sequencerColumnX + quarter * quarterWidth;
        int quarterY = sequencerColumnY;
        quarterRect.setBounds(quarterX, quarterY, quarterWidth, quarterHeight);
        quarterRect.removeFromLeft(3);
        quarterRect.removeFromRight(3);
        setQuarterBounds(quarterRect,quarter);
        quarterColums[quarter] = quarterRect;
    }
}

void SIP_by_RolandosAudioProcessorEditor::setQuarterBounds(juce::Rectangle<float> container,int quarter) {

    for (int row = 0; row < numRows; row++) {
        juce::Rectangle<float> quarterRowRect;
        int quarterRowWidth = container.getWidth();
        int quarterRowHeight = container.getHeight() / numRows;
        int rowX = container.getX();
        int rowY = container.getY() + row * quarterRowHeight;
        quarterRowRect.setBounds(rowX, rowY, quarterRowWidth, quarterRowHeight);
        
        setQuarterStepsBounds(quarterRowRect.withTrimmedLeft(2).withTrimmedRight(2), row, quarter);
        quarterRowRects[row][quarter] = quarterRowRect;
    };

}

void SIP_by_RolandosAudioProcessorEditor::setQuarterStepsBounds(juce::Rectangle<float> container, int row, int quarter) {
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

void SIP_by_RolandosAudioProcessorEditor::setStepBounds(juce::Rectangle<float> container, int row,int step) {
    juce::Rectangle<float> stepRect;
    int stepWidth = container.getWidth() - 5;
    int stepHeight = stepWidth;
    int stepX = container.getCentreX() - stepWidth*0.5;
    int stepY = container.getCentreY() - stepHeight * 0.5;
    stepRect.setBounds(stepX, stepY, stepWidth, stepHeight);
    stepRects[row][step] = stepRect;
}

void SIP_by_RolandosAudioProcessorEditor::setControlsBounds(juce::Rectangle<float> container) {
    int margin = 5;
    for (int row = 0; row < numRows; ++row) {
        juce::Rectangle<float> rowRect;
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

void SIP_by_RolandosAudioProcessorEditor::setControlBounds(juce::Rectangle<float> container,int row) {
    float margin = 2;
    juce::Rectangle<float> selectorContainer;
    float selectorContainerWidth = container.getWidth();
    float selectorContainerHeight = container.getHeight() * 0.3;
    float selectorContainerX = container.getX();
    float selectorContainerY = container.getY();
    selectorContainer.setBounds(selectorContainerX, selectorContainerY, selectorContainerWidth, selectorContainerHeight);
    selectorContainer.reduce(margin, margin);
    selectorContainers[row] = selectorContainer;
    if (instrSelectors.find(row) != instrSelectors.end()) {
        instrSelectors[row]->setBounds(selectorContainer.getSmallestIntegerContainer()); // Corrected
        addAndMakeVisible(instrSelectors[row]); // Add Slider to the editor

    }

    juce::Rectangle<float> attackContainer;
    float attackContainerWidth = container.getWidth()/3;
    float attackContainerHeight = container.getHeight() * 0.7;
    float attackContainerX = container.getX();
    float attackContainerY = container.getY() + selectorContainerHeight;
    attackContainer.setBounds(attackContainerX, attackContainerY, attackContainerWidth, attackContainerHeight);
    attackContainers[row] = attackContainer;
    if (attackSliders.find(row) != attackSliders.end()) {
        attackSliders[row]->setBounds(attackContainer.getSmallestIntegerContainer()); // Corrected
        addAndMakeVisible(attackSliders[row]); // Add Slider to the editor

    }


    juce::Rectangle<float> releaseContainer;
    float releaseContainerWidth = container.getWidth() / 3;
    float releaseContainerHeight = container.getHeight() * 0.7;
    float releaseContainerX = container.getX() + attackContainerWidth;
    float releaseContainerY = container.getY() + selectorContainerHeight;
    releaseContainer.setBounds(releaseContainerX, releaseContainerY, releaseContainerWidth, releaseContainerHeight);
    releaseContainers[row] = releaseContainer;
    if (releaseSliders.find(row) != releaseSliders.end()) {
        releaseSliders[row]->setBounds(releaseContainer.getSmallestIntegerContainer()); // Corrected
        addAndMakeVisible(releaseSliders[row]); // Add Slider to the editor
    }

    juce::Rectangle<float> cutoffContainer;
    float cutoffContainerWidth = container.getWidth() / 3;
    float cutoffContainerHeight = container.getHeight() * 0.7;
    float cutoffContainerX = releaseContainerX + releaseContainerWidth;
    float cutoffContainerY = container.getY() + selectorContainerHeight;
    cutoffContainer.setBounds(cutoffContainerX, cutoffContainerY, cutoffContainerWidth, cutoffContainerHeight);
    cutoffContainers[row] = cutoffContainer;
    if (cutoffSliders.find(row) != cutoffSliders.end()) {   
        cutoffSliders[row]->setBounds(cutoffContainer.getSmallestIntegerContainer()); // Corrected
        addAndMakeVisible(cutoffSliders[row]); // Add Slider to the editor
    }

}

void SIP_by_RolandosAudioProcessorEditor::setEffectsBounds(juce::Rectangle<float> container) {

    // Define the margin to reduce the size of the inner rectangle
    float margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<float> innerContainer = container.reduced(margin);
    juce::Rectangle<int> effectSelectorContainer;
    effectSelectorContainer.setBounds(innerContainer.getX(), innerContainer.getY(), innerContainer.getWidth(), innerContainer.getHeight() * 0.2);


    juce::Rectangle<float> knobsContainer;
    knobsContainer.setBounds(innerContainer.getX(), innerContainer.getY() + effectSelectorContainer.getHeight(), innerContainer.getWidth(), innerContainer.getHeight() * 0.8);

    effectSelectorContainer.reduce(margin, margin);
    effectSelector.setBounds(effectSelectorContainer);
    addAndMakeVisible(effectSelector);
    knobsContainer.reduce(margin, margin);
    setEffectKnobsBounds(knobsContainer);

}

void SIP_by_RolandosAudioProcessorEditor::setEffectKnobsBounds(juce::Rectangle<float> container) {
    // Define the margin to reduce the size of the inner rectangle
    float margin = 5;

    // Create a smaller rectangle inside the container bounds
    juce::Rectangle<float> innerContainer = container.reduced(margin);
    reverbContainer.setBounds(innerContainer.getX(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);


    delayContainer.setBounds(innerContainer.getX() + reverbContainer.getWidth(), innerContainer.getY(), innerContainer.getWidth() * 0.5, innerContainer.getHeight() * 0.5);
    reverbContainer.reduce(margin, margin);
    delayContainer.reduce(margin, margin);
    reverbSlider.setBounds(reverbContainer);
    addAndMakeVisible(reverbSlider);
    delaySlider.setBounds(delayContainer);
    addAndMakeVisible(delaySlider);

}

void SIP_by_RolandosAudioProcessorEditor::setKeyboardBounds(juce::Rectangle<float> container) {
    float gridBoundX = container.getX();
    float gridBoundY = container.getY();
    float gridBoundWidth = container.getWidth() * 0.75;
    float gridBoundHeight = container.getHeight();
    buttonGridBounds.setBounds(gridBoundX, gridBoundY, gridBoundWidth, gridBoundHeight);
    float sideButtonBoundsX = container.getX() + gridBoundWidth;
    float sideButtonBoundsY = container.getY() + (container.getHeight() * 0.5);
    float sideButtonBoundsWidth = container.getWidth() - gridBoundWidth;
    float sideButtonBoundsHeight = container.getHeight()* 0.5;
    sideButtonBounds.setBounds(sideButtonBoundsX, sideButtonBoundsY, sideButtonBoundsWidth, sideButtonBoundsHeight);

    // Draw the button grid in the gridBounds rectangle
    setButtonGridBounds(buttonGridBounds);

    // Draw the side buttons in the sideButtonBounds rectangle
    setSideButtonBounds(sideButtonBounds);
}

void SIP_by_RolandosAudioProcessorEditor::setButtonGridBounds(const juce::Rectangle<float>& gridBounds)
{
    float buttonSize = 40; // Define the size of each button
    float spacing = 15; // Define the spacing between buttons

    // Determine the number of rows and columns
    float numRows = 0;
    float numCols = 0;
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;
        if (button.row + 1 > numRows) numRows = button.row + 1;
        if (button.col + 1 > numCols) numCols = button.col + 1;
    }

    // Calculate the total size of the button grid
    float totalGridWidth = numCols * buttonSize + (numCols - 1) * spacing;
    float totalGridHeight = numRows * buttonSize + (numRows - 1) * spacing;

    // Calculate the offsets to center the grid
    float xOffset = (gridBounds.getWidth() - totalGridWidth) / 2;
    float yOffset = (gridBounds.getHeight() - totalGridHeight) / 2;

    // Iterate over buttons in the map and set their bounds
    for (const auto& entry : audioProcessor.keyButtonGrid)
    {
        const KeyButton& button = entry.second;

        // Calculate the position of the button based on row and column
        float xPos = gridBounds.getX() + xOffset + button.col * (buttonSize + spacing);
        float yPos = gridBounds.getY() + yOffset + button.row * (buttonSize + spacing);

        // Define the bounds for the button
        juce::Rectangle<float> buttonBounds(xPos, yPos, buttonSize, buttonSize);

        // Store the bounds in the buttonGridRects map
        buttonGridRects[entry.first] = buttonBounds;
    }
}

void SIP_by_RolandosAudioProcessorEditor::setSideButtonBounds(const juce::Rectangle<float>& sideButtonsContainer)
{
    float buttonSize = 50; // Define the size of each button
    float spacing = 10; // Define the spacing between buttons
    // Assuming we have two side buttons to draw
    for (const auto& entry : audioProcessor.sideButtons)
    {
        const KeyButton& button = entry.second;
        float xPos = sideButtonsContainer.getX() + button.col * (buttonSize + spacing);
        float yPos = sideButtonsContainer.getY() + button.row * (buttonSize + spacing);

        // Define the bounds for the button
        juce::Rectangle<float> buttonBounds(xPos, yPos, buttonSize, buttonSize);
        sideButtonsRects[entry.first] = buttonBounds;
    }
}

void SIP_by_RolandosAudioProcessorEditor::timerCallback()
{

    // Repaint the GUI
    paintView();
}

void SIP_by_RolandosAudioProcessorEditor::selectInstrument(int selectorIndex) {
    DBG("selectorIndex: ", selectorIndex);
    DBG("selectedInstr: ", instrSelectors[selectorIndex]->getSelectedId());
    audioProcessor.setSelectedInstrument(selectorIndex, instrSelectors[selectorIndex]->getSelectedId());

}

void SIP_by_RolandosAudioProcessorEditor::selectControl() {
    DBG("selected control: ", effectSelector.getSelectedId());
    audioProcessor.setSelectedControl(effectSelector.getText().toLowerCase());

}

