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
        //TODO: edge case (the user changes their sample rate mid session): change any logic that depends on sampleRate here
    }

}
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    resetParams(midiBuffer); //checks if user params have changed and updates class variables accordingly
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize; //usually ...128, 256, 512, 1024, currently 

    //test case for when i write unit tests
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        if (rhythms[i].interval * rhythms[i].subdivisions != samplesPerBar) {
            DBG("rhythm interval calculation error");
        }
    }
    

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        float temp = rhythms[i].interval * (rhythms[i].counter);
        rhythms[i].samplesProcessed += bufferSize;
        rhythmFlags[i] = (rhythms[i].samplesProcessed >= temp                         && rhythms[i].subdivisions > 1);


        if (rhythmFlags[i]) {
            if (apvts->getRawParameterValue("MACHINE" + to_string(i) + "." + to_string(rhythms[i].counter) + "_TOGGLE")->load() == true) {
                //TODO: sort out this bufferPosition calculation error. possibly related to GUI error?. possibly completely irrelevant variable
                int bufferPosition = totalSamples - temp;
                if (bufferPosition > bufferSize)
                {
                    DBG("midi buffer calculation error");
                    DBG(rhythms[i].interval);
                    DBG(totalSamples);
                    DBG(temp);
                }
                handleNoteTrigger(midiBuffer, rhythms[i].midiValue, bufferPosition);
            }
            rhythms[i].counter += 1;
        }
        if (rhythms[i].counter > rhythms[i].subdivisions) {
            rhythms[i].counter = 0;
            rhythms[i].samplesProcessed -= samplesPerBar;
        }
    }



    //do something with noteOffs
    if (totalSamples > samplesPerBar) {
        totalSamples -= samplesPerBar;
    }

}

void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber, int interval)
{
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    auto messageOff = juce::MidiMessage::noteOff(1, noteNumber);
    if (!midiBuffer.addEvent(message, interval))
    {
        DBG("error adding messages to midiBuffer");
    }
}


//TODO: clean up resetAll and resetParams so their purposes are more clear
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
  
    bpm = apvts->getRawParameterValue("BPM")->load();
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
        samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
        rhythms[i].interval = samplesPerBar / rhythms[i].subdivisions;
    }



}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new rhythm

    bpm = apvts->getRawParameterValue("BPM")->load();
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
        samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
        rhythms[i].interval = samplesPerBar / rhythms[i].subdivisions;
       
    }



}






/*
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{


    resetParams(midiBuffer); //checks if user params have changed and updates class variables accordingly
    auto bufferSize = buffer.getNumSamples();
    totalSamples += bufferSize;

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        rhythms[i].samplesProcessed = totalSamples % rhythms[i].interval;
    }


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
                        //DBG(midiIntToString(rhythms[i].midiValue));
                        //DBG(to_string(midiStringToInt(midiIntToString(rhythms[i].midiValue))));
                        //DBG("played note" + to_string(i) + "." + to_string(rhythms[i].counter));
                    }
                }
            }
            rhythms[i].counter += 1;
        }

    }

    //do something with noteOffs


}
*/