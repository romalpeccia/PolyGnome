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
        Metronome();

        void prepareToPlay(double _sampleRate, int samplesPerBlock);
        void getNextAudioBlock(juce::AudioBuffer<float>& buffer);
        void reset();



    private:
        int totalSamples = 0; //total samples since start time
        double sampleRate = 0; 
        int beatInterval = 0; //interval representing one beat click = (60.0 / bpm) * sampleRate
        double bpm = 240; 
        int samplesProcessed = 0; // samples processed before beat = totalSamples % interval

        juce::AudioFormatManager formatManager;
        std::unique_ptr <juce::AudioFormatReaderSource> pMetronomeSample =  nullptr ;
};