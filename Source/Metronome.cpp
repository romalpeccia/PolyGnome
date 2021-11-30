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

void Metronome::prepareToPlay(double tempSampleRate)
{
    sampleRate = tempSampleRate;
    interval = (60.0 / bpm) * sampleRate;
    
    HighResolutionTimer::startTimer(60.0);
        //abstract method?
}

void Metronome::hiResTimerCallback()
{
    interval = (60.0 / bpm) * sampleRate;
}

void Metronome::countSamples(int bufferSize)
{
    totalSamples += bufferSize;
    samplesRemaining = totalSamples % interval;
    //DBG(interval);
    DBG(samplesRemaining);
    if (samplesRemaining + bufferSize >= interval)
    {
        DBG("CLICK");
        DBG(totalSamples);

    }
    
}

void Metronome::reset() 
{
    totalSamples = 0;
}



