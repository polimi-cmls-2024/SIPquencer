/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIPquencerAudioProcessor::SIPquencerAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
        :AudioProcessor(BusesProperties()
            #if ! JucePlugin_IsMidiEffect
            #if ! JucePlugin_IsSynth
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
            #endif
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
            #endif
            )
#endif  
{
    ds.bindToPort(57121);
    sender.connectToSocket(ds, "127.0.0.1", 57121);
    isTempoRunning = false;
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
        sideButtons.insert({ key,KeyButton(key,i,0,defLabel,Rlabel,RPlabel,RRPlabel) });      
    };
}

SIPquencerAudioProcessor::~SIPquencerAudioProcessor()
{
 
}

//==============================================================================
const juce::String SIPquencerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SIPquencerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SIPquencerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SIPquencerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SIPquencerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SIPquencerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SIPquencerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SIPquencerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SIPquencerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SIPquencerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SIPquencerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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

void SIPquencerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SIPquencerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SIPquencerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
            isTempoRunning = true;

        }
        else if (message.isMidiStop())
        {
            DBG("Received MIDI STOP: " << message.getDescription());
            isTempoRunning = false;
         
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
            int value = message.getControllerValue();
            DBG(value);
            float normalizedValue = static_cast<float>(value) / 127.0f;

            switch (selectedControl) {
                 case Control::reverb:{
                    auto* reverbParameter = apvts.getParameter("reverb");
                    if (reverbParameter != nullptr) {
                        DBG(normalizedValue);
                        reverbParameter->setValueNotifyingHost(normalizedValue);
                        sender.send("/Control", 4, message.getControllerValue(), message.getChannel());
                    }
                    else {
                        DBG("Error: Reverb parameter not found in APVTS");
                    }
                    break;
                }
                      
                case Control::delay: {
                    auto* delayParameter = apvts.getParameter("delay");
                    if (delayParameter != nullptr) {
                        DBG(normalizedValue);
                        delayParameter->setValueNotifyingHost(normalizedValue);
                        sender.send("/Control", 5, message.getControllerValue(), message.getChannel());

                    }
                    else {
                        DBG("Error: Delay parameter not found in APVTS");
                    }
                    break;
                }
                case Control::attack:{
                    auto* attackParameter = apvts.getParameter("attack"+ std::to_string(selectedSequence + 1));
                    if (attackParameter != nullptr) {
                        DBG(normalizedValue);
                        attackParameter->setValueNotifyingHost(normalizedValue);
                        //sender.send("/Control", 2, message.getControllerValue(), message.getChannel());
                    }
                    else {
                        DBG("Error: attack parameter not found in APVTS");
                    }
                    break;
                }
                case Control::release: {
                    auto* releaseParameter = apvts.getParameter("release" + std::to_string(selectedSequence + 1));
                    if (releaseParameter != nullptr) {
                        DBG(normalizedValue);
                        releaseParameter->setValueNotifyingHost(normalizedValue);
                        //sender.send("/Control", 3, message.getControllerValue(), message.getChannel());

                    }
                    else {
                        DBG("Error: release parameter not found in APVTS");
                    }
                    break;
                }
                case Control::cutoff: {
                    auto* cutoffParameter = apvts.getParameter("cutoff" + std::to_string(selectedSequence + 1));
                    if (cutoffParameter != nullptr) {
                        DBG(normalizedValue);
                        cutoffParameter->setValueNotifyingHost(normalizedValue);
                        //sender.send("/Control", 1, message.getControllerValue(), message.getChannel());

                    }
                    else {
                        DBG("Error: release parameter not found in APVTS");
                    }
                    break;
                }
                default:
                   break;
            }
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
                break;
            case 6:
                updateBPM(data);
                break;
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
bool SIPquencerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SIPquencerAudioProcessor::createEditor()
{
    return new SIPquencerAudioProcessorEditor (*this);
}

//==============================================================================
void SIPquencerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
   /* auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);*/
}

void SIPquencerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
   /* std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));*/
}

juce::AudioProcessorValueTreeState::ParameterLayout SIPquencerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add the groups to the layout
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "attack1",  1 },
        "Attack",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));

    // Add the groups to the layout
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "attack2",  1 },
        "Attack",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));


    // Add the groups to the layout
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "attack3",  1 },
        "Attack",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "release1",  1 },
        "Release",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "release2",  1 },
        "Release",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "release3",  1 },
        "Release",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cutoff1",  1 },
        "Cutoff",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        2000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cutoff2",  1 },
        "Cutoff",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        2000.f));


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "cutoff3",  1 },
        "Cutoff",
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f),
        2000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>( 
        juce::ParameterID{ "reverb",  1 },
        "Reverb",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "delay",  1 },
        "Delay",
        juce::NormalisableRange<float>(0.001f, 1.f, 0.001f, 1.f),
        0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "bpm",  1 },
        "BPM",
        30.0f, 200.f, 120.0f));


    return layout;
}

        

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SIPquencerAudioProcessor();
}
// *****************************************************************************
void SIPquencerAudioProcessor::noteOn(const int midiKey)
{
    // Check if the MIDI key exists in the grid
    if (MIDItoKeyMap.find(midiKey) != MIDItoKeyMap.end())
    {
        // Update the button state
        keyButtonGrid[MIDItoKeyMap[midiKey]].playing = true;
    }
}

void SIPquencerAudioProcessor::noteOff(const int midiKey)
{
    // Check if the MIDI key exists in the grid
    if (MIDItoKeyMap.find(midiKey) != MIDItoKeyMap.end())
    {
        // Update the button state
        keyButtonGrid[MIDItoKeyMap[midiKey]].playing = false;
    }
}

void SIPquencerAudioProcessor::pressKey(const int key)
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

void SIPquencerAudioProcessor::releaseKey(int key)
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

void SIPquencerAudioProcessor::selectSequence(int seqNumber) {
    selectedSequence = seqNumber;
}

void SIPquencerAudioProcessor::selectStep(int stepNumber) {
	selectedStep = stepNumber;
}

void SIPquencerAudioProcessor::selectNote(int noteNumber) {
	selectedNote = noteNumber;
}

void SIPquencerAudioProcessor::updateSelectedSequence(const juce::uint8* data) {
    for (int i = 0; i < numSteps; ++i) {
        if (data[i + 1] < 128 && data[i + 1] >= 0) {
            sequences[selectedSequence][i] = data[i + 1];
        }
    }
}

void SIPquencerAudioProcessor::updateState(const unsigned char newState) {
	if (newState == DEF || newState == R || newState == RP || newState == RRP ) {
		state = newState;
	}
    else {
        DBG("Invalid state");
    }
}

void SIPquencerAudioProcessor::updateTranspose(const int transpose) {
    DBG("Transposing by: " << (transpose - transposeOffset));
    for (auto& entry : keyButtonGrid)
    {
        KeyButton& button = entry.second;
        //DBG("entry first (midi key): " << entry.first);
        button.midiKey = button.midiKey + (transpose - transposeOffset) * 12;
       // DBG("New key: " << button.midiKey);
        button.defLabel = juce::MidiMessage::getMidiNoteName(button.midiKey, true, true, 3);
        //DBG("New label: " << button.defLabel);
        button.RPlabel = button.defLabel;
    }
}

void SIPquencerAudioProcessor::setSelectedControl(const juce::String control) {
    auto cont = controlMap.find(control);
    if (cont != controlMap.end()) {
        selectedControl = cont->second;
    }
    else {
        DBG("control not found in controlMap");
    }
}

void SIPquencerAudioProcessor::setSelectedInstrument(int channel,int selectedInstr) {
    sender.send("/ProgramChange", channel, selectedInstr);
    DBG("channel"+ channel);
    DBG("selectedInstr"+ selectedInstr);

}

void SIPquencerAudioProcessor::updateBPM(int bpm) {
    BPM = bpm;
}

void SIPquencerAudioProcessor::sendControlChange(int paramIndex, float value ) {
    juce::String paramId = getParameterID(paramIndex);
    DBG("paramId: " + paramId);
    int intValue = static_cast<int>(value * 127.0f);
    DBG("intValue: " + std::to_string(intValue));

    if (paramId == "delay") {
		sender.send("/Control", 5, intValue, selectedSequence + 1);
        return;
    }
    else if(paramId == "reverb"){
        sender.send("/Control", 4, intValue, selectedSequence + 1);
        return;
    }
    else {
        // Get the last character of the parameter ID
        juce::String lastChar = paramId.substring(paramId.length() - 1);

        // Convert the last character to an integer
        int lastCharInt = lastChar.getIntValue();

        DBG("lastCharInt: " + std::to_string(lastCharInt));

        juce::String literalPart = paramId.dropLastCharacters(1);

        DBG("literalPart: " + literalPart);

        if (literalPart == "attack") {
            sender.send("/Control", 2, intValue, lastCharInt+1);
		}
		else if (literalPart == "release") {
			sender.send("/Control", 3, intValue, lastCharInt+1);
		}
		else if (literalPart == "cutoff") {
			sender.send("/Control", 1, intValue, lastCharInt+1);
        }

    }
    
}
 

