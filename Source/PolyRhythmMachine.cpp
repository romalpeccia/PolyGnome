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
    sampleRate = _sampleRate;
    resetParams();
}
void PolyRhythmMachine::getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    resetParams(midiBuffer); //checks if user params have changed and updates class variables accordingly
    auto bufferSize = buffer.getNumSamples(); //usually 16, 32, 64... 1024...
    totalSamples += bufferSize; 
    
    if (totalSamples > samplesPerBar) {
        int numBars = apvts->getRawParameterValue("NUM_BARS")->load();
        barCounter += 1;
        if (barCounter >= numBars){
            barCounter = 0;
        }
        apvts->getRawParameterValue("ACTIVE_BAR")->store(barCounter);
        if (apvts->getRawParameterValue("AUTO_LOOP")->load() == true) {
            apvts->getRawParameterValue("SELECTED_BAR")->store(barCounter);
        }

    }
    
    //TODO: old test case, possibly no longer relevant, double check
    /*
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (tracks[i].samplesPerInterval * tracks[i].subdivisions != samplesPerBar) {
            DBG("track samplesPerInterval calculation error");
        }
    }
    */

    for (int i = 0; i < MAX_TRACKS; i++) {

        if (apvts->getRawParameterValue("DAW_CONNECTED")->load() == false) {
            tracks[i].samplesProcessed += bufferSize;
        }
        else {
            tracks[i].samplesProcessed = ((int)apvts->getRawParameterValue("SAMPLES_ELAPSED")->load() % (int)samplesPerBar) + (int)bufferSize;
        }
        

        //turn any previously played notes off 
        float samplesAfterBeat = (((tracks[i].beatCounter - 1) * tracks[i].samplesPerInterval) + ((tracks[i].sustain / 100) * tracks[i].samplesPerInterval)); // the amount of samples for all intervals that have happened + the amount of samples after the latest interval
        if (tracks[i].samplesProcessed > samplesAfterBeat && tracks[i].noteOffFlag == true) {
            auto messageOff = juce::MidiMessage::noteOff(1, tracks[i].midiValue);
            midiBuffer.addEvent(messageOff, totalSamples - samplesAfterBeat);
            tracks[i].noteOffFlag = false;
            DBG("noteoff");
        }

        //reset the bar counter if it's been looped
        if (tracks[i].beatCounter > tracks[i].subdivisions) {
            tracks[i].beatCounter = 0;
                tracks[i].samplesProcessed -= samplesPerBar;
        }

        float samplesCounted = tracks[i].samplesPerInterval * (tracks[i].beatCounter);
        if (tracks[i].samplesProcessed >= samplesCounted) {
            if (tracks[i].beatCounter < MAX_SUBDIVISIONS) {
                if (apvts->getRawParameterValue(getBeatToggleString(barCounter, i, tracks[i].beatCounter))->load() == true ) {
                    
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
                    if (apvts->getRawParameterValue(getTrackEnableString(barCounter, i))->load() == true) {
                        handleNoteTrigger(midiBuffer, tracks[i].midiValue, tracks[i].velocity, bufferPosition);
                        tracks[i].noteOffFlag = true;
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

void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber, int velocity, int bufferPosition)
{
    auto message = juce::MidiMessage::noteOn(1, noteNumber, (juce::uint8)velocity);
    if (!midiBuffer.addEvent(message, bufferPosition))
    {
       DBG("error adding messages to midiBuffer");
    }

}


//TODO: clean up resetAll and resetParams so their purposes are more clear
void PolyRhythmMachine::resetAll()
{   //this should be called whenever the metronome is stopped
    totalSamples = 0;

    for (int i = 0; i < MAX_TRACKS; i++) {
        tracks[i].beatCounter = 0;
        tracks[i].samplesProcessed = 0;

    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  

    resetParams();
    int barNum = apvts->getRawParameterValue("SELECTED_BAR")->load();
    for (int i = 0; i < MAX_TRACKS; i++) {
        int tempMidiValue = apvts->getRawParameterValue(getMidiValueString(barNum, i))->load();
        if (tracks[i].midiValue != tempMidiValue)
        {
            auto messageOff = juce::MidiMessage::noteOff(1, tracks[i].midiValue);
            midiBuffer.addEvent(messageOff, 0);
            tracks[i].midiValue = tempMidiValue;
        }
    }
}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new track

    bpm = apvts->getRawParameterValue("BPM")->load();
    int barNum = apvts->getRawParameterValue("SELECTED_BAR")->load();
    for (int i = 0; i < MAX_TRACKS; i++) {
        int tempSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(barNum, i))->load();;
        if (tracks[i].subdivisions != tempSubdivisionValue)
        {

            auto oldRatio = (double)(tracks[i].beatCounter + 1.0) / (double)(tracks[i].subdivisions);
            /*
            DBG("i:" << i);
            DBG("tempSubdivisionValue" << tempSubdivisionValue);
            DBG("oldsubd: " << tracks[i].subdivisions);
            DBG("oldcounter: " <<  tracks[i].beatCounter);
            DBG("oldRatio:" << oldRatio);
            */
            tracks[i].subdivisions = tempSubdivisionValue;
            //solve for new tracks[i].beatCounter

            tracks[i].beatCounter = (int)(oldRatio * tracks[i].subdivisions);
            /*
            DBG("newsubd: " << tracks[i].subdivisions);
            DBG("newcounter: " << tracks[i].beatCounter);
            */

        }



        int tempVelocity = apvts->getRawParameterValue(getVelocityString(barNum, i))->load();
        if (tracks[i].velocity != tempVelocity) {
            tracks[i].velocity = tempVelocity;
        }

        int tempSustain = apvts->getRawParameterValue(getSustainString(barNum, i))->load();
        if (tracks[i].sustain != tempSustain) {
            tracks[i].sustain = tempSustain;
        }


        samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
        tracks[i].samplesPerInterval = samplesPerBar / tracks[i].subdivisions;
    }
}