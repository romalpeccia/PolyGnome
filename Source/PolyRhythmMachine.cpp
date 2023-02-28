/*
  ==============================================================================

    PolyRhythmMachine.cpp
    Created: 28 Feb 2023 10:35:26am
    Author:  romal

  ==============================================================================
*/

#include "PolyRhythmMachine.h"
#include <JuceHeader.h>

//==============================================================================
PolyRhythmMachine::PolyRhythmMachine()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}



PolyRhythmMachine::PolyRhythmMachine(juce::AudioProcessorValueTreeState* _apvts)
{
    apvts = _apvts;
    resetall();
}

PolyRhythmMachine::~PolyRhythmMachine()
{
}


void PolyRhythmMachine::prepareToPlay(double _sampleRate, int samplesPerBlock)
{
    //preparetoplay should call every time we start (right before)

    resetparams();

    if (sampleRate != _sampleRate)
    {
        //if the audioprocessors samplerate hasn't changed, nothing else needs to be done
        sampleRate = _sampleRate;
    }

}
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{


    resetparams();
    auto audioSourceChannelInfo = juce::AudioSourceChannelInfo(buffer);
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythms[i].samplesProcessed = totalSamples % rhythms[i].interval;
    }


    int rhythmFlags[MAX_MIDI_CHANNELS];

    //reset the rhythm counter
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythmFlags[i] = (rhythms[i].samplesProcessed + bufferSize >= rhythms[i].interval && rhythms[i].value > 1);
        if (rhythms[i].counter > rhythms[i].value) {
            rhythms[i].counter = 0;
            totalSamples = 0;

        }
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        if (rhythmFlags[i]) {

            rhythms[i].counter += 1;

            if (apvts->getRawParameterValue("MACHINE_RHYTHM1." + to_string(i) + "TOGGLE")->load() == true) {

                const auto timeToStartPlaying = rhythms[i].interval - rhythms[i].samplesProcessed;
                for (auto samplenum = 0; samplenum < bufferSize + 1; samplenum++)
                {
                    if (samplenum == timeToStartPlaying)
                    {
                        handleNoteTrigger(midiBuffer, rhythms[i].midiValue);
                    }
                }
            }

        }

    }


}

void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber)
{
    auto noteDuration = sampleRate;
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    //message.setTimeStamp(noteDuration);

    auto messageOff = juce::MidiMessage::noteOff(message.getChannel(), message.getNoteNumber());
    //messageOff.setTimeStamp((noteDuration));

    if (!midiBuffer.addEvent(message, 0) || !midiBuffer.addEvent(messageOff, 100))
    {
        DBG("error adding messages to midiBuffer");
    }

}
void PolyRhythmMachine::resetall()
{   //this should be called whenever the metronome is stopped
   // resetparams();
    totalSamples = 0;

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythms[i].counter = 0;
        rhythms[i].samplesProcessed = 0;

    }
}


void PolyRhythmMachine::resetparams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm



    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        int tempRhythmValue = 12;       // apvts->getRawParameterValue("  ")->load();
        if (rhythms[i].value != tempRhythmValue)
        {
            rhythms[i].value = tempRhythmValue;
            resetall();
        }
        rhythms[i].interval = 4 * ((60.0 / bpm) * sampleRate) / rhythms[i].value;
    }

    bpm = apvts->getRawParameterValue("BPM")->load();


}
