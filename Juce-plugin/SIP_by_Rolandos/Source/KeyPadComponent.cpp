/*
  ==============================================================================

    KeyPadComponent.cpp
    Created: 26 May 2024 1:50:02pm
    Author:  ricca

  ==============================================================================
*/

#include <JuceHeader.h>
#include "KeyPadComponent.h"
#include "PluginProcessor.h"


//==============================================================================
KeyPadComponent::KeyPadComponent(SIPquencerAudioProcessor& processor )
    :audioProcessor(processor)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

KeyPadComponent::~KeyPadComponent()
{
}

void KeyPadComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    paintButtonGrid(g);
    paintSideButtons(g);
   
}

void KeyPadComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    setKeyboardBounds(getLocalBounds().toFloat());

}



void KeyPadComponent::paintButtonGrid(juce::Graphics& gr) {
    for (const auto& entry : buttonGridRects)
    {
        const juce::Rectangle<float>& buttonRect = entry.second;
        const KeyButton& button = audioProcessor.keyButtonGrid.at(entry.first);
        drawButton(gr, buttonRect, button);
    }
}

void KeyPadComponent::paintSideButtons(juce::Graphics& gr) {
    for (const auto& entry : sideButtonsRects)
    {
        const juce::Rectangle<float>& buttonRect = entry.second;
        const KeyButton& button = audioProcessor.sideButtons.at(entry.first);

        drawSideButton(gr, buttonRect, button);
    }

}

void KeyPadComponent::drawButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, const KeyButton button)
{

    juce::Colour buttonColor = getButtonColor(button);

    /*
    juce::DropShadow buttonShdw;
    buttonShdw.colour = juce::Colours::black;
    buttonShdw.radius = 15;
    buttonShdw.offset = juce::Point(3, 3);

    buttonShdw.drawForRectangle(g, bounds.toNearestInt()
        .withTrimmedBottom(3)
        .withTrimmedLeft(3)
        .withTrimmedRight(3)
        .withTrimmedTop(3));
    */

    // Button shadow
    g.setColour(juce::Colours::darkgrey.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds.withPosition(bounds.getX(), bounds.getY() + 3), 5.f);


    // Draw button background
    g.setColour(buttonColor);
    g.fillRoundedRectangle(bounds, 5.f);
    g.setColour(juce::Colour(120, 120, 120));
    g.drawRoundedRectangle(bounds, 5.f, 0.5f);



    // Draw button label (for demonstration purposes)
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(juce::Font(16.0f));
    juce::String label = getButtonLabel(button);
    g.drawFittedText(label, bounds.getSmallestIntegerContainer(), juce::Justification::centred, 1);
}

void KeyPadComponent::drawSideButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, const KeyButton button)
{
    // Set color based on button state
    g.setColour(juce::Colours::darkgrey.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds.withPosition(bounds.getX(), bounds.getY() + 3), 13.f);


    // Set color based on button state
    juce::Colour buttonColor = getSideButtonColor(button);
    g.setColour(buttonColor);


    // Draw button background
    g.fillRoundedRectangle(bounds, 13.f);
    g.setColour(juce::Colour(120, 120, 120));
    g.drawRoundedRectangle(bounds, 13.f, 0.5f);


    // Draw button label (for demonstration purposes)
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(juce::Font(16.0f));
    juce::String label = getButtonLabel(button);
    g.drawFittedText(label, bounds.getSmallestIntegerContainer(), juce::Justification::centred, 1);
}


void KeyPadComponent::setKeyboardBounds(juce::Rectangle<float> container) {
    //float gridBoundX = container.getX();
    //float gridBoundY = container.getY();
    float gridBoundWidth = container.getWidth() * 0.75;
    //float gridBoundHeight = container.getHeight();
    //buttonGridBounds.setBounds(gridBoundX, gridBoundY, gridBoundWidth, gridBoundHeight);
    float sideButtonBoundsX = container.getX() + gridBoundWidth;
    float sideButtonBoundsY = container.getY() + (container.getHeight() * 0.5) + 15;
    float sideButtonBoundsWidth = container.getWidth() - gridBoundWidth;
    float sideButtonBoundsHeight = container.getHeight() * 0.5;
    sideButtonBounds.setBounds(sideButtonBoundsX, sideButtonBoundsY, sideButtonBoundsWidth, sideButtonBoundsHeight);

    // Draw the button grid in the gridBounds rectangle
    setButtonGridBounds(container);

    // Draw the side buttons in the sideButtonBounds rectangle
    setSideButtonBounds(sideButtonBounds);
}

void KeyPadComponent::setButtonGridBounds(const juce::Rectangle<float>& gridBounds)
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

void KeyPadComponent::setSideButtonBounds(const juce::Rectangle<float>& sideButtonsContainer)
{
    float buttonWidth = 45; // Define the size of each button
    float buttonHeight = 28;
    float spacing = 25; // Define the spacing between buttons
    // Assuming we have two side buttons to draw
    for (const auto& entry : audioProcessor.sideButtons)
    {
        const KeyButton& button = entry.second;
        float xPos = sideButtonsContainer.getX() + button.col * (buttonWidth + spacing);
        float yPos = sideButtonsContainer.getY() + button.row * (buttonHeight + spacing);

        // Define the bounds for the button
        juce::Rectangle<float> buttonBounds(xPos, yPos, buttonWidth, buttonHeight);
        sideButtonsRects[entry.first] = buttonBounds;
    }
}

juce::Colour KeyPadComponent::getButtonColor(const KeyButton button) const
{
    // Set color based on button state
    if (button.pressed) {
        return juce::Colours::darkgoldenrod;
    }
    else if (button.playing) {
        return juce::Colours::yellowgreen;
    }
    else {
        return juce::Colours::black;
        /* switch (audioProcessor.getSequencerState()) {
         case SIPquencerAudioProcessor::DEF:
             return juce::Colours::black;
         case SIPquencerAudioProcessor::R:
             return juce::Colours::dodgerblue;
         case SIPquencerAudioProcessor::RP:
             return juce::Colours::blanchedalmond;
         case SIPquencerAudioProcessor::RRP:
             return juce::Colours::coral;
         }*/
    }

}
juce::Colour KeyPadComponent::getSideButtonColor(const KeyButton button) const
{
    // Set color based on button state
    if (button.pressed) {
        return juce::Colours::darkgoldenrod;
    }
    else {

        return juce::Colour(192, 194, 192);

    }

}

juce::String KeyPadComponent::getButtonLabel(const KeyButton button) const
{
    switch (audioProcessor.getSequencerState()) {
    case SIPquencerAudioProcessor::DEF:
        return button.defLabel;
    case SIPquencerAudioProcessor::R:
        return button.Rlabel;
    case SIPquencerAudioProcessor::RP:
        return button.RPlabel;
    case SIPquencerAudioProcessor::RRP:
        return button.RRPlabel;
    default:
        DBG("Invalid state");
        return "Invalid state";
    }
}

