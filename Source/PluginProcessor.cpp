/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

using namespace std;
//==============================================================================
PolyGnomeAudioProcessor::PolyGnomeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

PolyGnomeAudioProcessor::~PolyGnomeAudioProcessor()
{
}


void PolyGnomeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    polyRhythmMachine.prepareToPlay(sampleRate, samplesPerBlock);
}

void PolyGnomeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void PolyGnomeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{


    auto positionInfo = getPlayHead()->getPosition();
    if (positionInfo) {
        auto bpmInfo = (*positionInfo).getBpm();
        auto timeInfo = (*positionInfo).getTimeInSamples();
        if (bpmInfo && timeInfo) {
            apvts.getRawParameterValue("BPM")->store(*bpmInfo);
            apvts.getRawParameterValue("SAMPLES_ELAPSED")->store(*timeInfo);
            apvts.getRawParameterValue("DAW_CONNECTED")->store(true);

            /*
            if (auto playingInfo = (*positionInfo).getIsPlaying()) {
                apvts.getRawParameterValue("ON/OFF")->store(true);
            }
            */
        }
        else {
            apvts.getRawParameterValue("DAW_CONNECTED")->store(false);
        }

    }
    
   
    midiMessages.clear(); //clear any noise in the midiBuffer
if (apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        polyRhythmMachine.getNextAudioBlock(buffer, midiMessages);
    }
}


juce::AudioProcessorValueTreeState::ParameterLayout PolyGnomeAudioProcessor::createParameterLayout() {
    //Creates all the parameters that change based on the user input and returns them in a AudioProcessorValueTreeState::ParameterLayout object

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //TODO: maybe use of format strings would make accessing these strings cleaner
    layout.add(std::make_unique<juce::AudioParameterBool>("ON/OFF", "On/Off", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("DAW_CONNECTED", "DAW Connected", false));
    layout.add(std::make_unique<juce::AudioParameterInt>("SAMPLES_ELAPSED", "samples elapsed", 0, 2147483647, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("NUM_RACKS", "num racks", 1, MAX_RACKS, 1));
    layout.add(std::make_unique<juce::AudioParameterInt>("SELECTED_RACK", "selected rack", 1, MAX_RACKS, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>("BPM", "bpm", juce::NormalisableRange<float>(1.f, 480.f, 0.1f, 0.25f), 120.f));


    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        //Parameters for Polytrack Machine 
        //BEAT_<0-MAX_MIDI_CHANNELS>.<0-MAX_TRACK_LENGTH>_TOGGLE
        //SUBDIVISIONS_<0-MAX_MIDI_CHANNELS>
        //MIDI_VALUE__<0-MAX_MIDI_CHANNELS>
        //TRACK_<0-MAX_MIDI_CHANNELS>_ENABLE
        for (int j = 0; j < MAX_TRACK_LENGTH; j++)
        {
            layout.add(std::make_unique<juce::AudioParameterBool>("BEAT_" + to_string(i) + "_" + to_string(j) + "_TOGGLE", "Beat" + to_string(i) + "." + to_string(j) + "Toggle", false));
        }
        layout.add(std::make_unique<juce::AudioParameterInt>("SUBDIVISIONS_" + to_string(i), "Subdivisions " + to_string(i), 1, MAX_TRACK_LENGTH, DEFAULT_SUBDIVISIONS));
        layout.add(std::make_unique<juce::AudioParameterInt>("MIDI_VALUE_" + to_string(i), "Midi Value " + to_string(i), 0, 127, DEFAULT_MIDI_VALUE + i));
        layout.add(std::make_unique<juce::AudioParameterInt>("VELOCITY_" + to_string(i), "Velocity " + to_string(i), 0, 127, DEFAULT_VELOCITY));
        layout.add(std::make_unique<juce::AudioParameterFloat>("SUSTAIN_" + to_string(i), "Sustain " + to_string(i), 0, 100.0, DEFAULT_SUSTAIN));
        layout.add(std::make_unique<juce::AudioParameterBool>("TRACK_" + to_string(i) + "_ENABLE", "Track " + to_string(i) + " Enable", true));
    }

    return layout;

}




juce::AudioProcessorEditor* PolyGnomeAudioProcessor::createEditor()
{
    //uncomment first return for generic sliders used for debugging purposes
    //return new juce::GenericAudioProcessorEditor(*this);

    return new PolyGnomeAudioProcessorEditor(*this);
}

//==============================================================================
void PolyGnomeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);

}
void PolyGnomeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.


    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
    
}




/*
******************************************
******************************************
Default unchanged JUCE library code
******************************************
*/


//==============================================================================
const juce::String PolyGnomeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PolyGnomeAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PolyGnomeAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PolyGnomeAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PolyGnomeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PolyGnomeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PolyGnomeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PolyGnomeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String PolyGnomeAudioProcessor::getProgramName(int index)
{
    return {};
}

void PolyGnomeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================


#ifndef JucePlugin_PreferredChannelConfigurations
bool PolyGnomeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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


//==============================================================================
bool PolyGnomeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolyGnomeAudioProcessor();
}

