/*
  ==============================================================================

    Metronome.cpp
    Created: 30 Nov 2021 12:18:05pm
    Author:  Romal
    d
  ==============================================================================
*/

#include "Metronome.h"
#include <JuceHeader.h>

void Metronome::countSamples(int bufferSize)
{
    totalSamples += bufferSize;
    DBG(totalSamples);
}

void Metronome::reset() 
{
    totalSamples = 0;
}

void Metronome::prepareToPlay(double tempSampleRate)
{
    sampleRate = tempSampleRate;
}