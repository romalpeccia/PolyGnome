/*
  ==============================================================================

    PolyRhythmMachine.cpp
    Created: 28 Feb 2023 10:35:26am
    Author:  romal

  ==============================================================================
*/

#include "PolyRhythmMachine.h"
#include <JuceHeader.h>
using namespace std;
//==============================================================================
PolyRhythmMachine::PolyRhythmMachine()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}



PolyRhythmMachine::PolyRhythmMachine(juce::AudioProcessorValueTreeState* _apvts)
{
    apvts = _apvts;
    resetAll();

}

PolyRhythmMachine::~PolyRhythmMachine()
{
}


void PolyRhythmMachine::prepareToPlay(double _sampleRate, int samplesPerBlock)
{
    //preparetoplay should call every time we start (right before)

    resetParams();

    if (sampleRate != _sampleRate)
    {
        sampleRate = _sampleRate;
        //change any logic that depends on sampleRate here
    }

}
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{


    resetParams(midiBuffer);
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythms[i].samplesProcessed = totalSamples % rhythms[i].interval;
    }

    //reset the rhythm counter
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythmFlags[i] = (rhythms[i].samplesProcessed + bufferSize >= rhythms[i].interval && rhythms[i].subdivisions > 1);
        if (rhythms[i].counter >= rhythms[i].subdivisions) {
            rhythms[i].counter = 0;
        }
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        if (rhythmFlags[i]) {
            if (apvts->getRawParameterValue("MACHINE" + to_string(i) + "." + to_string(rhythms[i].counter) + "_TOGGLE")->load() == true) {

                const auto timeToStartPlaying = rhythms[i].interval - rhythms[i].samplesProcessed;
                for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
                {
                    if (samplenum == timeToStartPlaying)
                    {
                        handleNoteTrigger(midiBuffer, rhythms[i].midiValue, rhythms[i].interval);
                        DBG(midiIntToString(rhythms[i].midiValue));
                        DBG(to_string(midiStringToInt(midiIntToString(rhythms[i].midiValue))));
                        //DBG("played note" + to_string(i) + "." + to_string(rhythms[i].counter));
                    }
                }
            }
            rhythms[i].counter += 1;
        }

    }


}

void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber, int interval)
{
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    auto messageOff = juce::MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());


    if (!midiBuffer.addEvent(messageOff, 0)|| !midiBuffer.addEvent(message, 0) )
    {
        DBG("error adding messages to midiBuffer");
    }
}
void PolyRhythmMachine::resetAll()
{   //this should be called whenever the metronome is stopped
   // resetParams();
    totalSamples = 0;

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythms[i].counter = 0;
        rhythms[i].samplesProcessed = 0;

    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm (TODO: maybe not? test this)
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        int tempRhythmValue = apvts->getRawParameterValue("MACHINE_SUBDIVISIONS" + to_string(i))->load();;
        if (rhythms[i].subdivisions != tempRhythmValue)
        {   
            rhythms[i].subdivisions = tempRhythmValue;
            resetAll();
        }
        int tempMidiValue = apvts->getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->load();
        if (rhythms[i].midiValue != tempMidiValue)
        {
            auto messageOff = juce::MidiMessage::noteOff(1, rhythms[i].midiValue);
            midiBuffer.addEvent(messageOff, 0);
            rhythms[i].midiValue = tempMidiValue;

            resetAll();
        }
        rhythms[i].interval = 4 * ((60.0 / bpm) * sampleRate) / rhythms[i].subdivisions;

    }
    bpm = apvts->getRawParameterValue("BPM")->load();


}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        int tempRhythmValue = apvts->getRawParameterValue("MACHINE_SUBDIVISIONS" + to_string(i))->load();;
        if (rhythms[i].subdivisions != tempRhythmValue)
        {
            rhythms[i].subdivisions = tempRhythmValue;
            resetAll();
        }
        int tempMidiValue = apvts->getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->load();
        if (rhythms[i].midiValue != tempMidiValue)
        {
            rhythms[i].midiValue = tempMidiValue;
            resetAll();
        }
        rhythms[i].interval = 4 * ((60.0 / bpm) * sampleRate) / rhythms[i].subdivisions;

    }
    bpm = apvts->getRawParameterValue("BPM")->load();


}

