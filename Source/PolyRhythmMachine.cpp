/*(
  ==============================================================================

    This file handles the output of MIDI notes to the user's DAW based on
    parameters selected in the UI.

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
            float samplesAfterBeat = (((bars[j].tracks[i].beatCounter - 1) * bars[j].tracks[i].samplesPerInterval) + 
                ((bars[j].tracks[i].sustain / 100) * bars[j].tracks[i].samplesPerInterval)); // the amount of samples for all intervals that have happened + the amount of samples after the latest interval
            if (j == barCounter) {
                //turn off any queued notes
                if (samplesProcessed >= samplesAfterBeat && bars[j].tracks[i].noteOffQueued == true) {
                    int noteOffIndex = (bars[j].tracks[i].beatCounter - 1 >= 0) ? bars[j].tracks[i].beatCounter - 1 :0 ;
                    auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[j].tracks[i].beats[noteOffIndex].midiValue + MIDI_STANDARD_OFFSET);
                    midiBuffer.addEvent(messageOff, samplesProcessed - samplesAfterBeat);
                    bars[j].tracks[i].noteOffQueued = false;
                }
            }
            float samplesCountedSinceBarStart = bars[j].tracks[i].samplesPerInterval * (bars[j].tracks[i].beatCounter);
            if (samplesProcessed >= samplesCountedSinceBarStart) {
                //place the correct MIDI on the buffer 
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
                                //if ((isDawConnected && isDawPlaying) || !isDawConnected) {
                                    //send the MIDI note that triggered
                                    handleNoteTrigger(midiBuffer, bars[j].tracks[i].beats[bars[j].tracks[i].beatCounter].midiValue, bars[j].tracks[i].velocity, bufferPosition);
                                    bars[j].tracks[i].noteOffQueued = true;
                               // }
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
            int newSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(j, i))->load();
            if (bars[j].tracks[i].subdivisions != newSubdivisionValue)
            {
                bars[j].tracks[i].subdivisions = newSubdivisionValue;
            }
            int newVelocity = apvts->getRawParameterValue(getVelocityString(j, i))->load();
            if (bars[j].tracks[i].velocity != newVelocity) {
                bars[j].tracks[i].velocity = newVelocity;
            }

            int newSustain = apvts->getRawParameterValue(getSustainString(j, i))->load();
            if (bars[j].tracks[i].sustain != newSustain) {
                bars[j].tracks[i].sustain = newSustain;
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
    for (int j = 0; j < MAX_BARS; j++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            bars[j].tracks[i].beatCounter = 0;

        }
    }
}


void PolyRhythmMachine::resetParams(juce::MidiBuffer& midiBuffer)
{  //this should be called when slider params change in UI to reflect changes in logic
   //this overloaded version allows you to send note offs for any notes currently playing, which is needed if the user changes a MIDI value while the app is running 
  //TODO: split these into onChange type functions for each parameter value? maybe apvts has some listener mechanism I can use
  //TODO: split MIDI handling function into seperate function


    for (int i = 0; i < MAX_TRACKS; i++) {
        for (int j = 0; j < MAX_SUBDIVISIONS; j++) {
            int newMidiValue = apvts->getRawParameterValue(getBeatMidiString(barCounter, i, j))->load();
            if (bars[barCounter].tracks[i].beats[j].midiValue != newMidiValue)
            {
                auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[barCounter].tracks[i].beats[j].midiValue + MIDI_STANDARD_OFFSET);
                midiBuffer.addEvent(messageOff, 0);
                bars[barCounter].tracks[i].beats[j].midiValue = newMidiValue;
            }
        }

        int selectedBar = apvts->getRawParameterValue("SELECTED_BAR")->load();
        int newSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(selectedBar, i))->load();
        if (newSubdivisionValue != bars[selectedBar].tracks[i].subdivisions) {
            for (int j = 0; j < MAX_SUBDIVISIONS; j++) {
                auto messageOff = juce::MidiMessage::noteOff(MIDI_CHANNEL, bars[selectedBar].tracks[i].beats[j].midiValue + MIDI_STANDARD_OFFSET);
                midiBuffer.addEvent(messageOff, 0);
            }

        }
        
    }
    resetParams();
}

void PolyRhythmMachine::resetParams()
{  //this should be called when params change in UI to reflect changes in logic
   //the variables keeping track of time should be reset to reflect the new track
    //TODO: split these into onChange type functions for each parameter value? maybe apvts has some listener mechanism I can use
    bpm = apvts->getRawParameterValue("BPM")->load();
    samplesPerBar = 4 * (60.0 / bpm) * sampleRate;
    for (int barNum = 0; barNum < MAX_BARS; barNum++) {
        for (int i = 0; i < MAX_TRACKS; i++) {

            int newSubdivisionValue = apvts->getRawParameterValue(getSubdivisionsString(barNum, i))->load();
            if (bars[barNum].tracks[i].subdivisions != newSubdivisionValue)
            {
                auto oldRatio = (double)(bars[barNum].tracks[i].beatCounter + 1.0) / (double)(bars[barNum].tracks[i].subdivisions);
                bars[barNum].tracks[i].subdivisions = newSubdivisionValue;
                //solve for new tracks[i].beatCounter
                bars[barNum].tracks[i].beatCounter = (int)(oldRatio * bars[barNum].tracks[i].subdivisions);
            }

            int newVelocity = apvts->getRawParameterValue(getVelocityString(barNum, i))->load();
            if (bars[barNum].tracks[i].velocity != newVelocity) {
                bars[barNum].tracks[i].velocity = newVelocity;
            }

            int newSustain = apvts->getRawParameterValue(getSustainString(barNum, i))->load();
            if (bars[barNum].tracks[i].sustain != newSustain) {
                bars[barNum].tracks[i].sustain = newSustain;
            }

            bars[barNum].tracks[i].samplesPerInterval = samplesPerBar / bars[barNum].tracks[i].subdivisions;
        }


    }
    

}