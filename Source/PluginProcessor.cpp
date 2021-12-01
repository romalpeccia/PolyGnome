/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MetroGnomeAudioProcessor::MetroGnomeAudioProcessor()
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
}

MetroGnomeAudioProcessor::~MetroGnomeAudioProcessor()
{
}

//==============================================================================
const juce::String MetroGnomeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MetroGnomeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MetroGnomeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MetroGnomeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MetroGnomeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MetroGnomeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MetroGnomeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MetroGnomeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MetroGnomeAudioProcessor::getProgramName (int index)
{
    return {};
}

void MetroGnomeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MetroGnomeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    metronome.prepareToPlay(sampleRate, samplesPerBlock);
}

void MetroGnomeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MetroGnomeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MetroGnomeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
   // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
   //     buffer.clear (i, 0, buffer.getNumSamples());



    bool playState = apvts.getRawParameterValue("ON/OFF")->load();
    if (playState== true)
    {
        metronome.getNextAudioBlock (buffer);
    }

}

//==============================================================================
bool MetroGnomeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MetroGnomeAudioProcessor::createEditor()
{
    return new MetroGnomeAudioProcessorEditor (*this);
}

//==============================================================================
void MetroGnomeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MetroGnomeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetroGnomeAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout MetroGnomeAudioProcessor::createParameterLayout() {
    //initializes parameter layout

    juce::AudioProcessorValueTreeState::ParameterLayout layout;


    layout.add(std::make_unique<juce::AudioParameterBool>("ON/OFF", "On/Off", false ));

    return layout;

}