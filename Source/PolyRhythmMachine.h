/*(
  ==============================================================================

    This file defines the PolyRhythmMachine class, which sends MIDI notes to
    the user's DAW based on parameters selected in the UI.

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
#pragma once
#include <JuceHeader.h>
#include "Utilities.h"

using namespace std;
//==============================================================================
/*
*/

struct Track {
    int samplesPerInterval = 0; //samples per subdivided beat, updated when user changes a param. samplesPerInterval =  4 * ((60.0 / bpm) * sampleRate) / tracks[i].subdivisions;
    
    int beatCounter = 0; // counts by 1 after each subdivided beat has been played (from 0 to num subdivisions) then resets to 0. 

    //user adjustable values
    int subdivisions = DEFAULT_SUBDIVISIONS;
    int midiValue = DEFAULT_MIDI_VALUE; 
    int velocity = DEFAULT_VELOCITY;
    float sustain = DEFAULT_SUSTAIN; //noteOffInterval = sustain * samplesPerInterval - 1
    bool noteOffQueued = false;
    bool barComplete = false;
};        
struct Bar {
    Track tracks[MAX_TRACKS];
};

class PolyRhythmMachine : public juce::Component
{
public:
    PolyRhythmMachine();
    PolyRhythmMachine(juce::AudioProcessorValueTreeState* _apvts);
    ~PolyRhythmMachine() override;

    void prepareToPlay(double _sampleRate, int samplesPerBlock);
    void getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer);
    void resetAll();
    void resetParams();
    void resetParams(juce::MidiBuffer& midiBuffer); 
    int getTotalSamples() { return samplesProcessed; }
    void handleBarChange(juce::MidiBuffer& midiBuffer);

    Bar bars[MAX_BARS]; 
    int barCounter = 0;
private:

    void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer&, int noteNumber, int velocity, int bufferPosition);
    int samplesProcessed = 0; //total samples since start time
    double sampleRate = 0; //sampleRate from DAW, usually 44100 samples/beat
    double samplesPerBar = 0; //= 4 * (60.0 / bpm) * sampleRate;
    double bpm = 120;


    //apvts of caller that created this instance of polyRhythmMachine
    juce::AudioProcessorValueTreeState* apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyRhythmMachine)
};

