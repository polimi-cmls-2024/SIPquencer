/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIP_by_RolandosAudioProcessor::SIP_by_RolandosAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
        :AudioProcessor(BusesProperties()
            #if ! JucePlugin_IsMidiEffect
            #if ! JucePlugin_IsSynth
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
            #endif
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
            #endif
            ),
    apvts(*this,nullptr,"Parameters",param::createParameterLayout()),
    params()
#endif  
{
    ds.bindToPort(57121);
    sender.connectToSocket(ds, "127.0.0.1", 57121);
    for (auto i = 0; i < param::NumParams; ++i)
    {
        const auto pID = static_cast<param::PID>(i);
        const auto id = param::toID(pID);
        params[i] = apvts.getParameter(id);
    }
    // Initialize the array to zeros
    for (int i = 0; i < seqRows; ++i) {
        for (int j = 0; j < numSteps; ++j) {
            sequences[i][j] = 0; // Assign zero to each element
        }
    }

    for (int i = 0; i < btnGridRows; ++i)
    {
        for (int j = 0; j < btnGridCols; ++j)
        {
            int index = i * btnGridCols + j;
            int midiKey = midiKeyScheme[index];
            int key = keys[index];
            MIDItoKeyMap.insert({ midiKey,key });
            juce::String defLabel = juce::MidiMessage::getMidiNoteName(midiKey, true, true, 3);
            juce::String Rlabel = RstateKeyLabels[index];
            juce::String RPlabel = defLabel;
            juce::String RRPlabel = RRPstateKeyLabels[index];

            keyButtonGrid.insert({ key,KeyButton(midiKey, i, j,defLabel,Rlabel,RPlabel,RRPlabel)});
        }
    }

    for(int i = 0; i< sideKeys.size(); ++i){
		int key = sideKeys[i];
		juce::String defLabel = defSideLables[i];
        juce::String Rlabel = RstateSideLabels[i];
        juce::String RPlabel = RPstateSideLabels[i];
        juce::String RRPlabel = RRPstateSideLabels[i];
        sideButtons.insert({ key,KeyButton(key,i,1,defLabel,Rlabel,RPlabel,RRPlabel) });      
    };
}

SIP_by_RolandosAudioProcessor::~SIP_by_RolandosAudioProcessor()
{
   
}

//==============================================================================
const juce::String SIP_by_RolandosAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SIP_by_RolandosAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SIP_by_RolandosAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SIP_by_RolandosAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SIP_by_RolandosAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SIP_by_RolandosAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SIP_by_RolandosAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SIP_by_RolandosAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SIP_by_RolandosAudioProcessor::getProgramName (int index)
{
    return {};
}

void SIP_by_RolandosAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SIP_by_RolandosAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // *****************************************************************************
    // 11. In prepare to play we need to reset our filter and initialize the filter
    // EX: void prepare (const ProcessSpec &) noexcept
    // It takes as input a ProcessSpec structure, where used sampleRate and samplesPerBloc
    // need to be specified


    // *****************************************************************************

}

void SIP_by_RolandosAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SIP_by_RolandosAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SIP_by_RolandosAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Iterate over MIDI messages
    for (const auto metadata : midiMessages)
    {
        const juce::MidiMessage& message = metadata.getMessage();
        //DBG("Received MIDI message: " << message.getDescription());
        // Check if the message is a MIDI_START
       

        // Check if the message is a MIDI_CLOCK
        if (message.isMidiClock())
        {

            // Increment the clock count
            clockCount++;

            // Calculate the current step based on clock count and BPM
            currentStep = (clockCount * numStepsPerQuarterNote) / clockPulsesPerQuarterNote;

            // Ensure currentStep stays within range
            currentStep %= numSteps;
            //DBG("Currentstep: " << currentStep);
          
        }
        else if (message.isMidiStart())
        {
            //DBG("Received MIDI START: " << message.getDescription());

            // Start tempo
            currentStep = 0; // Reset current step
            clockCount = 0; // Reset clock count

        }
        else if (message.isMidiStop())
        {
            DBG("Received MIDI STOP: " << message.getDescription());
         
            // Stop tempo
        }
        else if (message.isNoteOn()) {
            DBG("Received MIDI Note ON: " << message.getNoteNumber());
            const int note = message.getNoteNumber();
            sender.send("/NoteOn", message.getNoteNumber(), message.getVelocity(), message.getChannel());

            noteOn(message.getNoteNumber());

        }
		else if (message.isNoteOff()) {
            DBG("Received MIDI Note OFF: " << message.getNoteNumber());
            int note = message.getNoteNumber();
            sender.send("/NoteOff", message.getNoteNumber(), message.getChannel());

            noteOff(message.getNoteNumber());
		}
        else if (message.isController()) {
            DBG("Received MIDI controller: " << message.getDescription());
            sender.send("/Control", message.getControllerValue(), message.getControllerNumber());

        }
        else if (message.isSysEx()) {
          
          //DBG("Received MIDI SysEx: " << message.getDescription());
          const int commandType = message.getSysExData()[0];
          const int data = message.getSysExData()[1];


          DBG("command: " << commandType);
          DBG("data: " << data);
         
          switch (commandType) {
            case 0:
                if (data >= 0 && data < 12 || data==17 || data ==18) {
                    pressKey((int)data);
                }
                else if (data >= 50 && data < 62 || data == 67 || data == 68) {
                    releaseKey(data - 50);
                }
                break;
            case 1:
                if (data >= 0 && data < seqRows) {
                    selectSequence(data);
                }
                break;
            case 2:
                if (data >= 0 && data < numSteps) {
					selectStep(data);
				}
				break;
            case 3:
                updateSelectedSequence(message.getSysExData());  
                break;
            case 4:
                updateState(data);
                break;

            case 5:
                updateTranspose(data);
            default:
                break;
              
          }
          
        }
        if (currentStep >= numSteps)
        {
            currentStep = 0;
        }
    }

    // Clear MIDI buffer
    midiMessages.clear();

}


//==============================================================================
bool SIP_by_RolandosAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SIP_by_RolandosAudioProcessor::createEditor()
{
    return new SIP_by_RolandosAudioProcessorEditor (*this);
}

//==============================================================================
void SIP_by_RolandosAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SIP_by_RolandosAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SIP_by_RolandosAudioProcessor();
}
// *****************************************************************************
void SIP_by_RolandosAudioProcessor::noteOn(const int midiKey)
{
    // Check if the MIDI key exists in the grid
    if (MIDItoKeyMap.find(midiKey) != MIDItoKeyMap.end())
    {
        // Update the button state
        keyButtonGrid[MIDItoKeyMap[midiKey]].playing = true;
    }
}

void SIP_by_RolandosAudioProcessor::noteOff(const int midiKey)
{
    // Check if the MIDI key exists in the grid
    if (MIDItoKeyMap.find(midiKey) != MIDItoKeyMap.end())
    {
        // Update the button state
        keyButtonGrid[MIDItoKeyMap[midiKey]].playing = false;
    }
}

void SIP_by_RolandosAudioProcessor::pressKey(const int key)
{
    //DBG("Pressing key: " << key);
	// Check if the key exists in the grid
	if (key <12 && keyButtonGrid.find(key) != keyButtonGrid.end())
	{
		// Update the button state
        keyButtonGrid[key].pressed = true;
	}else if(key==17 || key==18 && sideButtons.find(key) != sideButtons.end()){
		sideButtons[key].pressed = true;
	}
	else {
		DBG("Invalid key");
	}
}


void SIP_by_RolandosAudioProcessor::releaseKey(int key)
{
    // Check if the key exists in the grid
   // DBG("Releasing key: " << key);

    if (key < 12 && keyButtonGrid.find(key) != keyButtonGrid.end())
    {
        // Update the button state
        keyButtonGrid[key].pressed = false;
    }
    else if (key == 17 || key == 18 && sideButtons.find(key) != sideButtons.end()) {
        sideButtons[key].pressed = false;
    }
    else {
        DBG("Invalid key");
    }
}

void SIP_by_RolandosAudioProcessor::selectSequence(int seqNumber) {
    selectedSequence = seqNumber;
}

void SIP_by_RolandosAudioProcessor::selectStep(int stepNumber) {
	selectedStep = stepNumber;
}

void SIP_by_RolandosAudioProcessor::selectNote(int noteNumber) {
	selectedNote = noteNumber;
}

void SIP_by_RolandosAudioProcessor::updateSelectedSequence(const juce::uint8* data) {
    for (int i = 0; i < numSteps; ++i) {
        if (data[i + 1] < 128 && data[i + 1] >= 0) {
            sequences[selectedSequence][i] = data[i + 1];
        }
    }
}

void SIP_by_RolandosAudioProcessor::updateState(const unsigned char newState) {
	if (newState == DEF || newState == R || newState == RP || newState == RRP ) {
		state = newState;
	}
    else {
        DBG("Invalid state");
    }
}

void SIP_by_RolandosAudioProcessor::updateTranspose(const int transpose) {
    DBG("Transposing by: " << (transpose - transposeOffset));
    for (auto& entry : keyButtonGrid)
    {
        KeyButton& button = entry.second;
        //DBG("entry first (midi key): " << entry.first);
        button.midiKey = entry.first + (transpose - transposeOffset) * 12;
       // DBG("New key: " << button.midiKey);
        button.defLabel = juce::MidiMessage::getMidiNoteName(button.midiKey, true, true, 3);
        //DBG("New label: " << button.defLabel);
        button.RPlabel = button.defLabel;
    }
}