/*
  ==============================================================================

    PolyRhythmMachine.h
    Created: 28 Feb 2023 10:35:26am
    Author:  romal

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Constants.h"

using namespace std;
//==============================================================================
/*
*/

struct Rhythm {
    int interval = 0;
    int samplesProcessed = 0;
    int counter = 0;
    int value = 1;
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
    void resetall();
    void resetparams();
    int getTotalSamples() { return totalSamples; }





private:

    void PolyRhythmMachine::handleNoteTrigger(juce::MidiBuffer&, int noteNumber);

    int totalSamples = 0; //total samples since start time
    double sampleRate = 0; //sampleRate from app, usually 44100
    double bpm = 60;
    Rhythm rhythms[MAX_MIDI_CHANNELS];

    //apvts of caller that created this instance of polyrhythmmetronome
    juce::AudioProcessorValueTreeState* apvts;


    const double startTime = juce::Time::getMillisecondCounterHiRes();

    //
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyRhythmMachine)
};

