/*
  ==============================================================================

    KeyButton.h
    Created: 26 May 2024 2:07:02pm
    Author:  ricca

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct KeyButton
{
    int midiKey;
    bool pressed;
    bool playing;
    int row;
    int col;
    juce::String defLabel;
    juce::String Rlabel;
    juce::String RPlabel;
    juce::String RRPlabel;
    KeyButton() : midiKey(0), pressed(false), playing(false), row(0), col(0), defLabel(""), Rlabel(""), RPlabel(""), RRPlabel("") {}
    KeyButton(int _midiKey, juce::String defLabel, juce::String _Rlabel, juce::String _RPlabel, juce::String _RRPlabel) :
        midiKey(_midiKey), pressed(false), playing(false), row(-1), col(-1),
        defLabel(defLabel), Rlabel(_Rlabel), RPlabel(_RPlabel), RRPlabel(_RRPlabel) {}
    KeyButton(int _midiKey, int _row, int _col, juce::String defLabel, juce::String _Rlabel, juce::String _RPlabel, juce::String _RRPlabel) :
        midiKey(_midiKey), pressed(false), playing(false), row(_row), col(_col),
        defLabel(defLabel), Rlabel(_Rlabel), RPlabel(_RPlabel), RRPlabel(_RRPlabel) {}
};