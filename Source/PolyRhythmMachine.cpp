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
        //TODO: rare edge case (the user changes their sample rate mid session): change any logic that depends on sampleRate here
    }

}
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    resetParams(midiBuffer); //checks if user params have changed and updates class variables accordingly
    auto bufferSize = buffer.getNumSamples(); //usually 16, 32, 64... 1024...
    totalSamples += bufferSize; 

    //test case. this may trigger when user changes the subdivisions during play time
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        if (tracks[i].samplesPerInterval * tracks[i].subdivisions != samplesPerBar) {
            DBG("track samplesPerInterval calculation error");
        }
    }
    

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        float samplesCounted = tracks[i].samplesPerInterval * (tracks[i].beatCounter);
        tracks[i].samplesProcessed += bufferSize;
        trackFlags[i] = (tracks[i].samplesProcessed >= samplesCounted     );

        if (tracks[i].beatCounter > tracks[i].subdivisions) {
            tracks[i].beatCounter = 0;
            tracks[i].samplesProcessed -= samplesPerBar;
        }
        if (trackFlags[i]) {
            if (tracks[i].beatCounter < MAX_MIDI_CHANNELS) {
                if (apvts->getRawParameterValue("BEAT_" + to_string(i) + "_" + to_string(tracks[i].beatCounter) + "_TOGGLE")->load() == true ) {
                    
                    //TODO: sort out this bufferPosition calculation error. possibly related to GUI error?. possibly completely irrelevant variable


                    int bufferPosition = totalSamples - samplesCounted; //TODO: maybe this should be samplesProcessed instead of totalSamples
                    //bufferPosition = 0; //for debugging
                    if (bufferPosition > bufferSize)
                    {
                        /*
                        DBG("midi buffer calculation error");
                        DBG(tracks[i].samplesPerInterval);
                        DBG(totalSamples);
                        DBG(samplesCounted);
                        */
                        ;
                    }
                    if (apvts->getRawParameterValue("TRACK_" + to_string(i) + "_ENABLE")->load() == true) {
                        handleNoteTrigger(midiBuffer, tracks[i].midiValue, bufferPosition);
                    }
                    else {
                        auto messageOff = juce::MidiMessage::noteOff(1, tracks[i].midiValue);
                        midiBuffer.addEvent(messageOff, 0);
                    }

                }
            }
            tracks[i].beatCounter += 1;
        }
    }

    if (totalSamples > samplesPerBar) {
        totalSamples -= samplesPerBar;
    }

}

void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber, int samplesPerInterval)
{
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)100);
    auto messageOff = juce::MidiMessage::noteOff(1, noteNumber);
    if (!midiBuffer.addEvent(message, samplesPerInterval))
    {
       DBG("error adding messages to midiBuffer");
    }
}


//TODO: clean up resetAll and resetParams so their purposes are more clear
void PolyRhythmMachine::resetAll()
{   //this should be called whenever the metronome is stopped
    totalSamples = 0;

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        tracks[i].beatCounter = 0;
        tracks[i].samplesProcessed = 0;

    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  
    bpm = apvts->getRawParameterValue("BPM")->load();
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        int tempRhythmValue = apvts->getRawParameterValue("SUBDIVISIONS_" + to_string(i))->load();;
        if (tracks[i].subdivisions != tempRhythmValue)
        {

            auto oldRatio = (double)(tracks[i].beatCounter + 1.0)/ (double)(tracks[i].subdivisions);
            /*
            DBG("i:" << i);
            DBG("tempRhythmValue" << tempRhythmValue);
            DBG("oldsubd: " << tracks[i].subdivisions);
            DBG("oldcounter: " <<  tracks[i].beatCounter);
            DBG("oldRatio:" << oldRatio);
            */
            tracks[i].subdivisions = tempRhythmValue;
            //solve for new tracks[i].beatCounter
            
            tracks[i].beatCounter = (int)(oldRatio * tracks[i].subdivisions);
            /*
            DBG("newsubd: " << tracks[i].subdivisions);
            DBG("newcounter: " << tracks[i].beatCounter);
            */

        }
        int tempMidiValue = apvts->getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
        if (tracks[i].midiValue != tempMidiValue)
        {
            auto messageOff = juce::MidiMessage::noteOff(1, tracks[i].midiValue);
            midiBuffer.addEvent(messageOff, 0);
            tracks[i].midiValue = tempMidiValue;
        }
        samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
        tracks[i].samplesPerInterval = samplesPerBar / tracks[i].subdivisions;
    }



}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new track
   //TODO: maybe this gets deleted in favor of the overloaded version

    bpm = apvts->getRawParameterValue("BPM")->load();
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        int tempRhythmValue = apvts->getRawParameterValue("SUBDIVISIONS_" + to_string(i))->load();;
        if (tracks[i].subdivisions != tempRhythmValue)
        {
            tracks[i].subdivisions = tempRhythmValue;
            //resetAll();
        }
        int tempMidiValue = apvts->getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
        if (tracks[i].midiValue != tempMidiValue)
        {
            tracks[i].midiValue = tempMidiValue;
            //resetAll();
        }
        samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
        tracks[i].samplesPerInterval = samplesPerBar / tracks[i].subdivisions;
       
    }
}