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
        int beatflag = subdivisions;

        int subInterval = 0;
        int subdivisions = 3; // TODO make this value a param


        int numerator = 4; //TODO make this a param
        int oneflag = numerator; // make this the numerator of the time sig on init

        double bpm = 60; 
        int samplesProcessed = 0; // samples processed before beat = totalSamples % interval

        juce::AudioFormatManager formatManager;

        //TODO code for loading multiple files
        //static const int numWAVs = 2;
        //std::array <std::unique_ptr <juce::AudioFormatReaderSource>, numWAVs> pMetronomeSamples;


        std::unique_ptr <juce::AudioFormatReaderSource> rimShotLow = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotHigh = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotSub = nullptr;
};