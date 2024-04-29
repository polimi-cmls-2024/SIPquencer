/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIP_sequencerAudioProcessorEditor::SIP_sequencerAudioProcessorEditor (SIP_sequencerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setSize (400, 300);
    //Frequency label
    addAndMakeVisible(&freqLabel);
    freqLabel.setFont(juce::Font(16.0f,juce::Font::bold));
    freqLabel.setColour (juce::Label::textColourId, juce::Colours::lightgreen);
    freqLabel.setJustificationType (juce::Justification::left);
    reverb.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgreen);
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    //Reverb slider & reverb label
    addAndMakeVisible(&reverb);
    addAndMakeVisible(&reverbLabel);
    reverb.setRange(0.0f, 1.0f);
    reverb.addListener(this);
    reverbLabel.setText("Reverberation:",juce::dontSendNotification);
    reverbLabel.attachToComponent(&reverb, true);
    reverb.setValue(0.33f);
}

SIP_sequencerAudioProcessorEditor::~SIP_sequencerAudioProcessorEditor()
{
}

//==============================================================================
void SIP_sequencerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    freqLabel.setText("Frequency: " + juce::String(processor.freq)+" Hz", juce::dontSendNotification);
    
}

void SIP_sequencerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto sliderLeft = 120;
    freqLabel.setBounds(20, 50, 200, 30);
    reverb.setBounds(sliderLeft,100, 250, 30);
}

void SIP_sequencerAudioProcessorEditor::setLabelText(float freqHz)
{
    freqLabel.setText(juce::String(),juce::dontSendNotification);
}

void SIP_sequencerAudioProcessorEditor::sliderValueChanged(juce::Slider *slider)
{
    processor.sendReverb(reverb.getValue());
}
