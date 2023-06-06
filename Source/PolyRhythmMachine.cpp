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
    resetParams(midiBuffer); //checks if apvts params have changed and updates class variables accordingly
    
    
    auto bufferSize = buffer.getNumSamples(); //usually 16, 32, 64... 1024...
    totalSamples = (int)apvts->getRawParameterValue("SAMPLES_ELAPSED")->load() % (int)samplesPerBar;

    //TODO: old test case, possibly no longer relevant, double check
    /*
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (tracks[i].samplesPerInterval * tracks[i].subdivisions != samplesPerBar) {
            DBG("track samplesPerInterval calculation error");
        }
    }
    */
    
    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {

            if (apvts->getRawParameterValue("DAW_CONNECTED")->load() == false) {
                bars[j].tracks[i].samplesProcessed += bufferSize;
            }
            else {
                bars[j].tracks[i].samplesProcessed = ((int)apvts->getRawParameterValue("SAMPLES_ELAPSED")->load() % (int)samplesPerBar) + (int)bufferSize;
            }


            //turn any previously played notes off 
            float samplesAfterBeat = (((bars[j].tracks[i].beatCounter - 1) * bars[j].tracks[i].samplesPerInterval) + ((bars[j].tracks[i].sustain / 100) * bars[j].tracks[i].samplesPerInterval)); // the amount of samples for all intervals that have happened + the amount of samples after the latest interval
            if (j == barCounter) {
                if (bars[j].tracks[i].samplesProcessed > samplesAfterBeat && bars[j].tracks[i].noteOffQueued == true) {
                    auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[j].tracks[i].midiValue + TEMP_MIDI_BUGFIX_NUM);
                    midiBuffer.addEvent(messageOff, totalSamples - samplesAfterBeat);
                    bars[j].tracks[i].noteOffQueued = false;
                }
            }


            //reset the beat counters if it's been looped
            if (bars[j].tracks[i].beatCounter > bars[j].tracks[i].subdivisions) {
                bars[j].tracks[i].beatCounter = 0;
                bars[j].tracks[i].samplesProcessed -= samplesPerBar;
            }

            float samplesCounted = bars[j].tracks[i].samplesPerInterval * (bars[j].tracks[i].beatCounter);
            if (bars[j].tracks[i].samplesProcessed >= samplesCounted) {

                if (j == barCounter) {
                    if (bars[j].tracks[i].beatCounter < MAX_SUBDIVISIONS) {
                        if (apvts->getRawParameterValue(getBeatToggleString(barCounter, i, bars[j].tracks[i].beatCounter))->load() == true) {

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
                                handleNoteTrigger(midiBuffer, bars[j].tracks[i].midiValue, bars[j].tracks[i].velocity, bufferPosition);
                                bars[j].tracks[i].noteOffQueued = true;
                            }
                        }
                    }
                }
                bars[j].tracks[i].beatCounter += 1;
            }
        }
    }


    if (totalSamples + bufferSize > samplesPerBar) {
        totalSamples -= samplesPerBar;
            handleBarChange(midiBuffer);
    }

}

void PolyRhythmMachine::handleBarChange(juce::MidiBuffer& midiBuffer) {
    int numBars = apvts->getRawParameterValue("NUM_BARS")->load();
    barCounter += 1;
    if (barCounter >= numBars) {
        barCounter = 0;
    }

    resetParams(midiBuffer);
    apvts->getRawParameterValue("ACTIVE_BAR")->store(barCounter);
    if (apvts->getRawParameterValue("AUTO_LOOP")->load() == true) {
        apvts->getRawParameterValue("SELECTED_BAR")->store(barCounter);
    }

    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            bars[j].tracks[i].beatCounter = 0;
            //tracks[i].samplesProcessed -= samplesPerBar;
            /*
            int tempSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(j, i))->load();
            if (bars[j].tracks[i].subdivisions != tempSubdivisionValue)
            {
                bars[j].tracks[i].subdivisions = tempSubdivisionValue;
            }
            int tempVelocity = apvts->getRawParameterValue(getVelocityString(j, i))->load();
            if (bars[j].tracks[i].velocity != tempVelocity) {
                bars[j].tracks[i].velocity = tempVelocity;
            }

            int tempSustain = apvts->getRawParameterValue(getSustainString(j, i))->load();
            if (bars[j].tracks[i].sustain != tempSustain) {
                bars[j].tracks[i].sustain = tempSustain;
            }
            */
        }

    }


}


void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer& midiBuffer, int noteNumber, int velocity, int bufferPosition)
{
    auto message = juce::MidiMessage::noteOn(MIDI_CHANNEL, noteNumber + TEMP_MIDI_BUGFIX_NUM, (juce::uint8)velocity);
    if (!midiBuffer.addEvent(message, bufferPosition))
    {
       DBG("error adding messages to midiBuffer");
    }

}



void PolyRhythmMachine::resetAll()
{   //this should be called whenever the metronome is stopped
    totalSamples = 0;
    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            bars[j].tracks[i].beatCounter = 0;
            bars[j].tracks[i].samplesProcessed = 0;
        }
    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  

    resetParams();
    for (int i = 0; i < MAX_TRACKS; i++) {
        int tempMidiValue = apvts->getRawParameterValue(getMidiValueString(barCounter, i))->load();
        if (bars[barCounter].tracks[i].midiValue != tempMidiValue)
        {
            auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[barCounter].tracks[i].midiValue + TEMP_MIDI_BUGFIX_NUM);
            midiBuffer.addEvent(messageOff, 0);
            bars[barCounter].tracks[i].midiValue = tempMidiValue;
        }
    }
    

}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new track

    bpm = apvts->getRawParameterValue("BPM")->load();
    samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
    for (int barNum = 0; barNum < MAX_BARS; barNum++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            int tempSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(barNum, i))->load();;
            if (bars[barNum].tracks[i].subdivisions != tempSubdivisionValue)
            {

                auto oldRatio = (double)(bars[barNum].tracks[i].beatCounter + 1.0) / (double)(bars[barNum].tracks[i].subdivisions);
                /*
                DBG("i:" << i);
                DBG("tempSubdivisionValue" << tempSubdivisionValue);
                DBG("oldsubd: " << tracks[i].subdivisions);
                DBG("oldcounter: " <<  tracks[i].beatCounter);
                DBG("oldRatio:" << oldRatio);
                */
                bars[barNum].tracks[i].subdivisions = tempSubdivisionValue;
                //solve for new tracks[i].beatCounter

                bars[barNum].tracks[i].beatCounter = (int)(oldRatio * bars[barNum].tracks[i].subdivisions);
                /*
                DBG("newsubd: " << tracks[i].subdivisions);
                DBG("newcounter: " << tracks[i].beatCounter);
                */

            }
            int tempVelocity = apvts->getRawParameterValue(getVelocityString(barNum, i))->load();
            if (bars[barNum].tracks[i].velocity != tempVelocity) {
                bars[barNum].tracks[i].velocity = tempVelocity;
            }

            int tempSustain = apvts->getRawParameterValue(getSustainString(barNum, i))->load();
            if (bars[barNum].tracks[i].sustain != tempSustain) {
                bars[barNum].tracks[i].sustain = tempSustain;
            }
            bars[barNum].tracks[i].samplesPerInterval = samplesPerBar / bars[barNum].tracks[i].subdivisions;
        }


    }
    

}