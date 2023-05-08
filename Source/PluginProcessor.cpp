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
MetroGnomeAudioProcessor::MetroGnomeAudioProcessor()
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

MetroGnomeAudioProcessor::~MetroGnomeAudioProcessor()
{
}


void MetroGnomeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    metronome.prepareToPlay(sampleRate, samplesPerBlock);
    polyRhythmMetronome.prepareToPlay(sampleRate, samplesPerBlock);
    polyRhythmMachine.prepareToPlay(sampleRate, samplesPerBlock);
}

void MetroGnomeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


void MetroGnomeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
   
    //check if the user is in a recognized DAW or other host
    if (juce::String(pluginHostType.getHostDescription()) != "Unknown") {
        //apvts.getRawParameterValue("HOST_CONNECTED")->store(true);
        /*
        if (double bpm = playHead->getPosition()->getBpm() != 0) {
            
        }
        */
    }
   
    midiMessages.clear();
    auto mode = apvts.getRawParameterValue("MODE")->load();

    if (apvts.getRawParameterValue("ON/OFF")->load() == true && mode == 0)
    {
        metronome.getNextAudioBlock(buffer);
    }
    else if (apvts.getRawParameterValue("ON/OFF")->load() == true && mode == 1)
    {
        polyRhythmMetronome.getNextAudioBlock(buffer, midiMessages);
    }
    else if (apvts.getRawParameterValue("ON/OFF")->load() == true && mode == 3)
    {
        polyRhythmMachine.getNextAudioBlock(buffer, midiMessages);
    }
}


juce::AudioProcessorValueTreeState::ParameterLayout MetroGnomeAudioProcessor::createParameterLayout() {
    //Creates all the parameters that change based on the user input and stores them in a apvts.layout object

    juce::AudioProcessorValueTreeState::ParameterLayout layout;



    layout.add(std::make_unique<juce::AudioParameterBool>("ON/OFF", "On/Off", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("HOST_CONNECTED", "Host Connected", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("BPM", "bpm", juce::NormalisableRange<float>(1.f, 300.f, 0.1f, 0.25f), 120.f));
    layout.add(std::make_unique<juce::AudioParameterInt>("SUBDIVISION", "Subdivision", 1, MAX_LENGTH, 1));
    layout.add(std::make_unique<juce::AudioParameterInt>("NUMERATOR", "Numerator", 1, MAX_LENGTH, 4));


    juce::StringArray stringArray;
    stringArray.add("Default");
    stringArray.add("Polyrhythm");
    stringArray.add("Polymeter");
    stringArray.add("Machine");
    layout.add(std::make_unique<juce::AudioParameterChoice>("MODE", "Mode", stringArray, 0));

    for (int i = 0; i < MAX_LENGTH; i++) {
        layout.add(std::make_unique<juce::AudioParameterBool>("RHYTHM1."+ to_string(i) + "_TOGGLE", "Rhythm1." + to_string(i) + " Toggle", false));
        layout.add(std::make_unique<juce::AudioParameterBool>("RHYTHM2." + to_string(i) + "_TOGGLE", "Rhythm2." + to_string(i) + " Toggle", false));
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
    {
        for (int j = 0; j < MAX_LENGTH; j++)
        {
            layout.add(std::make_unique<juce::AudioParameterBool>("MACHINE" + to_string(i) + "." + to_string(j) + "_TOGGLE", "Machine" + to_string(i) + "." + to_string(j) + "Toggle", false));
        }
        layout.add(std::make_unique<juce::AudioParameterInt>("MACHINE_SUBDIVISIONS" + to_string(i), "Machine Subdivisions" + to_string(i), 1, MAX_LENGTH, 1));
        layout.add(std::make_unique<juce::AudioParameterInt>("MACHINE_MIDI_VALUE" + to_string(i), "Machine Midi" + to_string(i), 0, 127, 36 + i));

    }

    return layout;

}




juce::AudioProcessorEditor* MetroGnomeAudioProcessor::createEditor()
{
    //uncomment first return for generic sliders used for debugging purposes
    //return new juce::GenericAudioProcessorEditor(*this);

    return new MetroGnomeAudioProcessorEditor(*this);
}

//==============================================================================
void MetroGnomeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);

}
void MetroGnomeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    /*TODO: UNCOMMENT TO RESTORE params when the plugin is closed
    * 
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
    */
}




/*
******************************************
******************************************
Default unchanged JUCE library code
******************************************
*/


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

void MetroGnomeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String MetroGnomeAudioProcessor::getProgramName(int index)
{
    return {};
}

void MetroGnomeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================


#ifndef JucePlugin_PreferredChannelConfigurations
bool MetroGnomeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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
bool MetroGnomeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetroGnomeAudioProcessor();
}

