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
    bool isDawConnected = apvts->getRawParameterValue("DAW_CONNECTED")->load();
    bool isDawPlaying = apvts->getRawParameterValue("DAW_PLAYING")->load();
    
    auto bufferSize = buffer.getNumSamples(); //usually 16, 32, 64... 1024...

    if (!isDawConnected) {
        samplesProcessed += bufferSize;
    }
    else {
        samplesProcessed = ((int)apvts->getRawParameterValue("DAW_SAMPLES_ELAPSED")->load() % (int)samplesPerBar) + (int)bufferSize;
    }
    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            //turn any previously played notes off 
            float samplesAfterBeat = (((bars[j].tracks[i].beatCounter - 1) * bars[j].tracks[i].samplesPerInterval) + ((bars[j].tracks[i].sustain / 100) * bars[j].tracks[i].samplesPerInterval)); // the amount of samples for all intervals that have happened + the amount of samples after the latest interval
            if (j == barCounter) {
                if (samplesProcessed >= samplesAfterBeat && bars[j].tracks[i].noteOffQueued == true) {
                    auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[j].tracks[i].midiValue + MIDI_STANDARD_OFFSET);
                    midiBuffer.addEvent(messageOff, samplesProcessed - samplesAfterBeat);
                    bars[j].tracks[i].noteOffQueued = false;
                }
            }
            float samplesCountedSinceBarStart = bars[j].tracks[i].samplesPerInterval * (bars[j].tracks[i].beatCounter);
            if (samplesProcessed >= samplesCountedSinceBarStart) {

                if (j == barCounter) {
                    if (bars[j].tracks[i].beatCounter < MAX_SUBDIVISIONS) {
                        if (apvts->getRawParameterValue(getBeatToggleString(barCounter, i, bars[j].tracks[i].beatCounter))->load() == true) {

                            //TODO: sort out this bufferPosition calculation
                            int bufferPosition = samplesProcessed - samplesCountedSinceBarStart; 
                            //bufferPosition = 0; //for debugging
                            if (bufferPosition > bufferSize)
                            {
                                /*
                                DBG("midi buffer calculation error");
                                DBG(bars[j].tracks[i].samplesPerInterval);
                                DBG(samplesProcessed);
                                DBG(samplesCountedSinceBarStart);
                                */
                                
                            }
                            if (apvts->getRawParameterValue(getTrackEnableString(barCounter, i))->load() == true) {
                                if ((isDawConnected && isDawPlaying) || !isDawConnected) {
                                    //send the MIDI note that triggered
                                    handleNoteTrigger(midiBuffer, bars[j].tracks[i].midiValue, bars[j].tracks[i].velocity, bufferPosition);
                                    bars[j].tracks[i].noteOffQueued = true;
                                }
                            }
                        }
                    }
                }
                bars[j].tracks[i].beatCounter += 1;
            }
        }
    }


    if (samplesProcessed  >= samplesPerBar) {
        samplesProcessed -= samplesPerBar;
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
            //samplesProcessed -= samplesPerBar;
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
    auto message = juce::MidiMessage::noteOn(MIDI_CHANNEL, noteNumber + MIDI_STANDARD_OFFSET, (juce::uint8)velocity);
    if (!midiBuffer.addEvent(message, bufferPosition))
    {
       DBG("error adding messages to midiBuffer");
    }

}



void PolyRhythmMachine::resetAll()
{   //this should be called whenever the metronome is stopped
    samplesProcessed = 0;            
    samplesProcessed = 0;
    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            bars[j].tracks[i].beatCounter = 0;

        }
    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  
    for (int i = 0; i < MAX_TRACKS; i++) {

        int tempMidiValue = apvts->getRawParameterValue(getMidiValueString(barCounter, i))->load();
        if (bars[barCounter].tracks[i].midiValue != tempMidiValue)
        {
            auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[barCounter].tracks[i].midiValue + MIDI_STANDARD_OFFSET);
            midiBuffer.addEvent(messageOff, 0);
            bars[barCounter].tracks[i].midiValue = tempMidiValue;
        }

        int selectedBar = apvts->getRawParameterValue("SELECTED_BAR")->load();
        int tempSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(selectedBar, i))->load();
        if (tempSubdivisionValue != bars[selectedBar].tracks[i].subdivisions) {
            auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[selectedBar].tracks[i].midiValue + MIDI_STANDARD_OFFSET);
            midiBuffer.addEvent(messageOff, 0);
        }
        
    }
    resetParams();
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
                bars[barNum].tracks[i].subdivisions = tempSubdivisionValue;
                //solve for new tracks[i].beatCounter
                bars[barNum].tracks[i].beatCounter = (int)(oldRatio * bars[barNum].tracks[i].subdivisions);
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