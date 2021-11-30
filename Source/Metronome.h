/*
  ==============================================================================

    Metronome.h
    Created: 30 Nov 2021 12:18:05pm
    Author:  Romal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Metronome : public juce::HighResolutionTimer
{
    public:
        void prepareToPlay(double sampleRate);
        void countSamples(int bufferSize);
        void reset();
        void hiResTimerCallback() override;


    private:
        int totalSamples = 0; //total samples since start time
        double sampleRate = 0; 
        int interval; //interval representing one beat click = (60.0 / bpm) * sampleRate
        double bpm = 120; 
        int samplesRemaining; // samples processed before beat = totalSamples % interval
};