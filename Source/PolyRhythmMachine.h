/*
  ==============================================================================

    PolyRhythmMachine.h
    Created: 28 Feb 2023 10:35:26am
    Author:  romal

  ==============================================================================
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
    int samplesProcessed = 0; //TODO remove this, make it a single class variable
    int beatCounter = 0; // counts by 1 after each subdivided beat has been played (from 0 to num subdivisions) then resets to 0. 

    //user adjustable values
    int subdivisions = DEFAULT_SUBDIVISIONS;
    int midiValue = DEFAULT_MIDI_VALUE; 
    int velocity = DEFAULT_VELOCITY;
    float sustain = DEFAULT_SUSTAIN; //noteOffInterval = sustain * samplesPerInterval - 1
    bool noteOffFlag = false;
};        


class PolyRhythmMachine : public juce::Component
{
public:
    PolyRhythmMachine();
    PolyRhythmMachine(juce::AudioProcessorValueTreeState* _apvts);
    ~PolyRhythmMachine() override;

    void prepareToPlay(double _sampleRate, int samplesPerBlock);
    void getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer);// override; //no override?
    void resetAll();
    void resetParams();
    void resetParams(juce::MidiBuffer& midiBuffer); 
    int getTotalSamples() { return totalSamples; }

    Track tracks[MAX_TRACKS];

private:

    void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer&, int noteNumber, int velocity, int bufferPosition);

    int totalSamples = 0; //total samples since start time
    double sampleRate = 0; //sampleRate from DAW, usually 44100 samples/beat
    double samplesPerBar = 0; //= 4 * (60.0 / bpm) * sampleRate;
    double bpm = 120;
    int barCounter = 0;

    //apvts of caller that created this instance of polyRhythmMachine
    juce::AudioProcessorValueTreeState* apvts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyRhythmMachine)
};

