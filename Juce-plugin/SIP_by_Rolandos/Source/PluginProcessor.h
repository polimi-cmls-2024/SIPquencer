/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Params.h"
#include <array>
  

// *****************************************************************************
struct KeyButton
{
    int midiKey;
    bool pressed;
    bool playing;
    int row;
    int col;

    KeyButton() : midiKey(0), pressed(false), playing(false), row(0),col(0) {}
    KeyButton(int _midiKey,int _row, int _col) : midiKey(_midiKey), pressed(false),playing(false), row(_row), col(_col) {}
};
//==============================================================================
/**
*/
class SIP_by_RolandosAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SIP_by_RolandosAudioProcessor();
    ~SIP_by_RolandosAudioProcessor() override;

    static constexpr int numSteps = 16; // Define the number of steps
    static constexpr int seqRows = 3; // Define the number of sequences

    static constexpr int btnGridRows = 4;  // Define the number of rows in the button grid
    static constexpr int btnGridCols = 3;   // Define the number of cols in the button grid
    const std::array<int, btnGridRows* btnGridCols> midiKeyScheme = {
        60, 61, 62,   // Example MIDI keys for the button grid
        63, 64, 65,
        66, 67, 68,
        69, 70, 71
        };
    const std::array<int,12> keys = {1,2,3,4,5,6,7,8,9,10,0,11};


    std::map<int, KeyButton> keyButtonGrid;
    std::map<int, int> MIDItoKeyMap = {
		{60,1},
		{61,2},
		{62,3},
		{63,4},
		{64,5},
		{65,6},
		{66,7},
		{67,8},
		{68,9},
		{69,10},
		{70,0},
		{71,11}
	};




    using RAPPtr = juce::RangedAudioParameter*;
    using RAPPtrArray = std::array<RAPPtr, param::NumParams>;

   


    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    void noteOn(int midiKey);
    void noteOff(int midiKey);
    void pressKey(int keyCode);
    void releaseKey(int keyCode);

  
    void selectSequence(int seqNumber);
    void selectStep(int stepNumber);
    void selectNote(int noteNumber);

    int getCurrentStep() { return currentStep; }
    int getSelectedStep() { return selectedStep; }
    int getSequenceStep(int seq, int step) { return sequences[seq][step]; }
    int getSelectedSequence() { return selectedSequence; }
    // *****************************************************************************
    // 1. We are going to use the AudioProcessorValueTreeState class for implementing
    // the communication between Editor and Processor   
    //   // AudioProcessorValueTreeState
    juce::AudioProcessorValueTreeState apvts;
    RAPPtrArray params;  


    // we need to define a function where we actually associate all parameters and that
    // returns a ParameterLayout
    //juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // *****************************************************************************

private:
    //==============================================================================
      // Add member variables for tempo and step tracking
    bool isTempoRunning = false;

    int currentStep = 0;
    int clockCount = 0;
    int numStepsPerQuarterNote = 4;
    int clockPulsesPerQuarterNote = 96;

    int selectedSequence = 0;
    int selectedStep = 0;
    int selectedNote = 0;
    
    std::array<std::array<int,numSteps>,seqRows> sequences;
    

    // Other member variables...
    // *****************************************************************************
    // 9. Add the DSP filter in the Processor header
    // ProcessorDuplicator is a template that converst mono processor class in a multi-channel version
    // template<typename MonoProcessorType, typename StateType>	
    // we also add a variable lastSampleRate to support later

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_by_RolandosAudioProcessor)
};
