/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SIP_sequencerAudioProcessor::SIP_sequencerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    ds.bindToPort(57120);
    sender.connectToSocket(ds, "127.0.0.1", 57120);
    
}

SIP_sequencerAudioProcessor::~SIP_sequencerAudioProcessor()
{
    sender.disconnect();
}

//==============================================================================
const juce::String SIP_sequencerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SIP_sequencerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SIP_sequencerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SIP_sequencerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SIP_sequencerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SIP_sequencerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SIP_sequencerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SIP_sequencerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SIP_sequencerAudioProcessor::getProgramName (int index)
{
    return {};
}

void SIP_sequencerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SIP_sequencerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SIP_sequencerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SIP_sequencerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void SIP_sequencerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();
    juce::MidiBuffer processedMidi;
    int time;
    juce::MidiMessage m;
    for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
        {
            if (m.isNoteOn())
            {
                freq = (float) juce::MidiMessage::getMidiNoteInHertz(m.getNoteNumber());


                

               /* if (index<8){
                    
                    sequence[index] = freq;
                    index++*/;
                    sender.send("/FMSynth/freq", freq);
                //}
    
            }
            
        }
}

//==============================================================================
bool SIP_sequencerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SIP_sequencerAudioProcessor::createEditor()
{
    return new SIP_sequencerAudioProcessorEditor (*this);
}

//==============================================================================
void SIP_sequencerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SIP_sequencerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SIP_sequencerAudioProcessor::sendReverb(float reverbValue)
{
    sender.send("/FMSynth/reverb",(1.0f-reverbValue));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SIP_sequencerAudioProcessor();
}
