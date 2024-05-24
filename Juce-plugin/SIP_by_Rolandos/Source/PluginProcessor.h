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
    juce:: String defLabel;
    juce::String Rlabel;
    juce::String RPlabel;
    juce::String RRPlabel;
    KeyButton() : midiKey(0), pressed(false), playing(false), row(0),col(0) ,defLabel(""),Rlabel(""), RPlabel(""), RRPlabel("") {}
    KeyButton(int _midiKey, juce::String defLabel, juce::String _Rlabel, juce::String _RPlabel, juce::String _RRPlabel) :
        midiKey(_midiKey), pressed(false), playing(false), row(-1), col(-1),
        defLabel(defLabel), Rlabel(_Rlabel), RPlabel(_RPlabel), RRPlabel(_RRPlabel) {}
    KeyButton(int _midiKey,int _row, int _col,juce::String defLabel, juce::String _Rlabel, juce::String _RPlabel, juce::String _RRPlabel) :
        midiKey(_midiKey), pressed(false),playing(false), row(_row), col(_col),
        defLabel(defLabel),Rlabel(_Rlabel),RPlabel(_RPlabel),RRPlabel(_RRPlabel) {}
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

    enum State {
        DEF,  // Default state
        R,    // R state
        RP,   // RP state
        RRP   // RRP state
    };
    enum Control {
        attack,
        release,
        cutoff,
        delay,
        reverb
    };

    const std::map<juce::String, Control> controlMap = {
    {"attack", Control::attack},
    {"release", Control::release},
    {"cutoff", Control::cutoff},
    {"delay", Control::delay},
	{"reverb", Control::reverb}
    // Add more mappings here
    };
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
    const std::array<int, 2> sideKeys = {17,18};

    const std::array<juce::String, 2> defSideLables = { "oct+","oct-" };

    const std::array<juce::String, 12> RstateKeyLabels = { "1","2","3","4","5","6","7","8","9","Clear Seq.","0","Mute Seq." };
    const std::array<juce::String, 2> RstateSideLabels = { "step",""};

    const std::array<juce::String, 2> RPstateSideLabels = { "","note" };


    //const std::array<juce::String, 12> RPstateLabels = { "1","2","3","4","5","6","7","8","9","10","0","11" };
    const std::array<juce::String, 12> RRPstateKeyLabels = { "Seq. 1","Seq. 2","Seq.3","","","","","","Acid Maniac","Clear All","","Play/Pause"};
    const std::array<juce::String, 2> RRPstateSideLabels = { "seq.","view"};


    std::map<int, KeyButton> keyButtonGrid;
    std::map<int, KeyButton> sideButtons;
    std::map<int, int> MIDItoKeyMap;


    const int transposeOffset = 64;
    Control selectedControl;

 
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this,nullptr,"Parameters",createParameterLayout()};
    


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


    void noteOn(const int midiKey);
    void noteOff(const int midiKey);
    void pressKey(const int keyCode);
    void releaseKey(int keyCode);

  
    void selectSequence(int seqNumber);
    void selectStep(int stepNumber);
    void selectNote(int noteNumber);

    void updateSelectedSequence(const juce::uint8* data);
    void updateState(const unsigned char newState);

    int getCurrentStep() const { return currentStep; }
    int getSelectedStep()const { return selectedStep; }
    int getSequenceStep(int seq, int step) const { return sequences[seq][step]; }
    int getSelectedSequence() const { return selectedSequence; }
    unsigned char getSequencerState() const { return state; }
    int getBpm() { return BPM; };
    void updateTranspose(const int transpose);

    void setSelectedControl(const juce::String control);
    void setSelectedInstrument(int channel, int selectedId);
    void updateBPM(int bpm);


    const std::array<int, 2> getSideKeys() const { return sideKeys; }
    // *****************************************************************************
    // 1. We are going to use the AudioProcessorValueTreeState class for implementing
    // the communication between Editor and Processor   
    //   // AudioProcessorValueTreeState
 


    // we need to define a function where we actually associate all parameters and that
    // returns a ParameterLayout
    //juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // *****************************************************************************

private:
    //==============================================================================
      // Add member variables for tempo and step tracking

    bool isTempoRunning = false;
    int BPM;

    int currentStep = 0;
    int clockCount = 0;
    int numStepsPerQuarterNote = 4;
    int clockPulsesPerQuarterNote = 96;

    int selectedSequence = 0;
    int selectedStep = 0;
    int selectedNote = 0;
    
    std::array<std::array<int,numSteps>,seqRows> sequences;


 
    unsigned char state = DEF;

    juce::DatagramSocket ds;
    juce::OSCSender sender;

    // Other member variables...
    // *****************************************************************************
    // 9. Add the DSP filter in the Processor header
    // ProcessorDuplicator is a template that converst mono processor class in a multi-channel version
    // template<typename MonoProcessorType, typename StateType>	
    // we also add a variable lastSampleRate to support later

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIP_by_RolandosAudioProcessor)
};
