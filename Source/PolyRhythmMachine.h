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

struct Rhythm {
    int interval = 0; //samples per subdivided beat, updated when user changes a param. interval interval =  4 * ((60.0 / bpm) * sampleRate) / rhythms[i].subdivisions;
    int samplesProcessed = 0; 
    int counter = 0; // counts from 0 to num subdivisions then resets to 0
    int subdivisions = 1;
    int midiValue = 36; 
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

    Rhythm rhythms[MAX_MIDI_CHANNELS];
    bool rhythmFlags[MAX_MIDI_CHANNELS]; 

private:

    void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer&, int noteNumber, int interval);

    int totalSamples = 0; //total samples since start time
    double sampleRate = 0; //sampleRate from DAW, usually 44100 samples/beat
    double samplesPerBar = 0; //= 4 * (60.0 / bpm) * sampleRate;
    double bpm = 60;

    //apvts of caller that created this instance of polyRhythmMachine
    juce::AudioProcessorValueTreeState* apvts;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyRhythmMachine)
};

