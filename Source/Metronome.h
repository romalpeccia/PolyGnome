/*
  ==============================================================================

    Metronome.h
    Created: 30 Nov 2021 12:18:05pm
    Author:  Romal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Metronome
{
    public:
        void prepareToPlay(double sampleRate);
        void countSamples(int bufferSize);
        void reset();

    private:
        int totalSamples = 0;
        double sampleRate = 0;
};