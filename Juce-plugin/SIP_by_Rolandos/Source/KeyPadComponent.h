/*
  ==============================================================================

    KeyPadComponent.h
    Created: 26 May 2024 1:50:02pm
    Author:  ricca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KeyButton.h"
#include "PluginProcessor.h"
//==============================================================================
/*
*/
class KeyPadComponent  : public juce::Component
{
public:
    KeyPadComponent(SIPquencerAudioProcessor& processor);
    ~KeyPadComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void paintButtonGrid(juce::Graphics&);
    void paintSideButtons(juce::Graphics&);

    void drawButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, KeyButton button);
    void drawSideButton(juce::Graphics& g, const juce::Rectangle<float>& bounds, KeyButton button);
    void setKeyboardBounds(juce::Rectangle<float> container);
    void setButtonGridBounds(const juce::Rectangle<float>& gridBounds);
    void setSideButtonBounds(const juce::Rectangle<float>& sideButtonsContainer);


    juce::Colour getButtonColor(const KeyButton button) const;
    juce::Colour getSideButtonColor(const KeyButton button) const;
    juce::String getButtonLabel(const KeyButton button) const;
private:

    SIPquencerAudioProcessor& audioProcessor;

    juce::Rectangle<float> keysContainer;
    std::map<int, juce::Rectangle<float>> buttonGridRects;
    std::map<int, juce::Rectangle<float>> sideButtonsRects;
    juce::Rectangle<float> buttonGridBounds;
    juce::Rectangle<float> sideButtonBounds;

 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyPadComponent)
};
