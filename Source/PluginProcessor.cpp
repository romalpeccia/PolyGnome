/*(
  ==============================================================================

    This file handles the backend of the PolyGnome, such as the creation of 
    parameters, processing of parameters and DAW variables, and an instantion
    of the PolyRhythmMachine class to process timing of MIDI notes. 

  ==============================================================================
    Copyright(C) 2024 Romal Peccia

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>

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
    //get internal Bar state info from apvts
    int selectedBar = apvts.getRawParameterValue("SELECTED_BAR")->load();
    int activeBar = apvts.getRawParameterValue("ACTIVE_BAR")->load();
    int isAutoLoopEnabled = apvts.getRawParameterValue("AUTO_LOOP")->load();
    int numBars = apvts.getRawParameterValue("NUM_BARS")->load();
    if (selectedBar >= numBars) {
        apvts.getRawParameterValue("SELECTED_BAR")->store(numBars - 1);
    }
    if (selectedBar != activeBar && isAutoLoopEnabled) {
        apvts.getRawParameterValue("SELECTED_BAR")->store(activeBar);
    }

    //process info from the DAW
    auto positionInfo = getPlayHead()->getPosition();
    if (positionInfo) {
        apvts.getRawParameterValue("DAW_CONNECTED")->store(true);
        auto bpmInfo = (*positionInfo).getBpm();
        auto timeInfo = (*positionInfo).getTimeInSamples();
        auto isPlayingInfo = (*positionInfo).getIsPlaying();
        if (bpmInfo) {
            apvts.getRawParameterValue("BPM")->store(*bpmInfo);
        }
        if (timeInfo){
            apvts.getRawParameterValue("DAW_SAMPLES_ELAPSED")->store(*timeInfo);
        }

        if (isPlayingInfo != NULL) { //isPlayingInfo is NULL instead of false when the DAW is not playing
            apvts.getRawParameterValue("DAW_PLAYING")->store(isPlayingInfo);
             if ( resetBarAfterPause == false) {
                resetBarAfterPause = true;
                apvts.getRawParameterValue("ON/OFF")->store(true);
            }

        }
        else {
            apvts.getRawParameterValue("DAW_PLAYING")->store(false);
            if (resetBarAfterPause == true) {
                polyRhythmMachine.resetAll();
                resetBarAfterPause = false;
                apvts.getRawParameterValue("ON/OFF")->store(false);
            }
        }
    }




    //process incoming notes from keyboard for active miditexteditor
    int selectedMidi = apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->load();

    if (selectedMidi != -1){
        juce::MidiBuffer::Iterator iterator(midiMessages);
        juce::MidiMessage currentMessage;
        int samplePos;
        if (iterator.getNextEvent(currentMessage, samplePos)) {
            if (currentMessage.isNoteOn()) {
                DBG(currentMessage.getDescription());
                storedMidiFromKeyboard = currentMessage.getNoteNumber();
            }
        }
    }

    midiMessages.clear(); //clear any noise or notes inputted by user in the midiBuffer

    if (apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        //run the machine
        polyRhythmMachine.getNextAudioBlock(buffer, midiMessages);
        midiNeedsClearing = true;
    }
    else {
        //the machine has been stopped, clear all MIDI notes
        if (midiNeedsClearing == true) {
            for (int i = 0; i < MAX_MIDI_VALUE; i++) {
                auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, i);
                midiMessages.addEvent(messageOff, 0);
            }
            midiNeedsClearing = false;
        }
    }

    //process the MIDI added to the MIDIbuffer by polyRhythmMachine.getNextAudioBlock, add it to the keyboardState which later gets read by the keyBoardComponent
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), false);


}


juce::AudioProcessorValueTreeState::ParameterLayout PolyGnomeAudioProcessor::createParameterLayout() {
    //Creates all the parameters that change based on the user input and returns them in a AudioProcessorValueTreeState::ParameterLayout object

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterBool>("ON/OFF", "On/Off", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("AUTO_LOOP", "auto loop", false));

    layout.add(std::make_unique<juce::AudioParameterBool>("DAW_CONNECTED", "DAW Connected", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("DAW_PLAYING", "DAW Playing", false)); 
    layout.add(std::make_unique<juce::AudioParameterFloat>("BPM", "bpm", juce::NormalisableRange<float>(1.f, 480.f, 0.1f, 0.25f), 120.f));
    layout.add(std::make_unique<juce::AudioParameterInt>("DAW_SAMPLES_ELAPSED", "samples elapsed", 0, 2147483647, 0));

    layout.add(std::make_unique<juce::AudioParameterInt>("NUM_BARS", "num bars", 1, MAX_BARS, 1));
    layout.add(std::make_unique<juce::AudioParameterInt>("SELECTED_BAR", "selected bar", 0, MAX_BARS - 1, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("ACTIVE_BAR", "active bar", 0, MAX_BARS - 1, 0));

    layout.add(std::make_unique<juce::AudioParameterInt>("SELECTED_MIDI_TRACK", "selected midi", -1, MAX_TRACKS, -1));
    //Parameters for Polytrack Machine 
    //<0-MAX_BARS>_BEAT_<0-MAX_TRACKS>.<0-MAX_SUBDIVISIONS>_TOGGLE
    //<0-MAX_BARS>_SUBDIVISIONS_<0-MAX_TRACKS>
    //<0-MAX_BARS>_MIDI_VALUE_<0-MAX_TRACKS>
    //<0-MAX_BARS>_SUSTAIN_<0-MAX_TRACKS>
    //<0-MAX_BARS>_TRACK_<0-MAX_TRACKS>_ENABLE
    for (int barNum = 0; barNum < MAX_BARS; barNum++) {
        for (int i = 0; i < MAX_TRACKS; i++)
        {
            for (int j = 0; j < MAX_SUBDIVISIONS; j++)
            {
                    layout.add(std::make_unique<juce::AudioParameterBool>(getBeatToggleString(barNum, i, j), to_string(barNum) + "_Beat" + to_string(i) + "." + to_string(j) + "Toggle", false));
            }
            layout.add(std::make_unique<juce::AudioParameterInt>(getSubdivisionsString(barNum, i), to_string(barNum) + "_Subdivisions " + to_string(i), 1, MAX_SUBDIVISIONS, DEFAULT_SUBDIVISIONS));
            layout.add(std::make_unique<juce::AudioParameterInt>(getMidiValueString(barNum, i), to_string(barNum) + "_Midi Value " + to_string(i), 0, MAX_MIDI_VALUE, DEFAULT_MIDI_VALUE + i));
            layout.add(std::make_unique<juce::AudioParameterInt>(getVelocityString(barNum, i), to_string(barNum) + "_Velocity " + to_string(i), 0, MAX_VELOCITY, DEFAULT_VELOCITY));
            layout.add(std::make_unique<juce::AudioParameterFloat>(getSustainString(barNum, i), to_string(barNum) + "_Sustain " + to_string(i), 0, 100.0, DEFAULT_SUSTAIN));
            layout.add(std::make_unique<juce::AudioParameterBool>(getTrackEnableString(barNum, i), to_string(barNum) + "_Track " + to_string(i) + " Enable", true));
            }

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
    apvts.copyState().writeToStream(mos);

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
Default JUCE library code
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

