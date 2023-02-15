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
        Metronome(juce::AudioProcessorValueTreeState* _apvts);

        void prepareToPlay(double _sampleRate, int samplesPerBlock);
        void getNextAudioBlock(juce::AudioBuffer<float>& buffer);
        void resetall();
        void resetparams();
        int getNumerator() {return numerator;}
        int getSubdivisions() {return subdivisions;}
        int getBPM() { return bpm;}
        int getoneflag() { return oneflag;}
        int getbeatCount() {return beatCount;}
        float getSamplesProcessed() { return samplesProcessed; }
        float getSubSamplesProcessed() { return subSamplesProcessed; }

        /*
     sampleRate gives us the amount of samples (in our incoming audio buffers) per second
     we can calculate the amount of samples represent a beat
     beatInterval = (60.0 / bpm) * sampleRate;
     we can then calculate the amount of samples representing a subdivision of that beat
     subInterval = beatInterval / #subdivisions;
     every time we process an audio block we increment totalSamples by the amount of samples incoming
     totalSamples += bufferSize;
     samplesProcessed = totalSamples % beatInterval;
     if we have processed more samples than the amount of samples representing one beat (or one subdivision if subdivisions are turned on,
     we play a sound depending on what triggered the sound event
     main beat event
     regular beat event
     sub event

     based on beatCount, oneflag
     //beatCount keeps count of which subdivision we're on, +=1 when subdivision click is played, reset to 1 when main beat is finished
     //oneflag signals a first beat of bar when oneflag = numerator, +=1 every main beat, reset to 1 after a bar

     TODO rename beatCount, subflag, beatinterval, subinterval to something more descriptive
     */

        //User params, which change when the sliders are moved
        int numerator = 4; //numerator of time signature
        int subdivisions = 1; // amount of subdivisions, 1 = turns off subdivision logic 
        double bpm = 60;

        //overall logic variables
        int totalSamples = 0; //total samples since start time
        double sampleRate = 0;

        //beat logic variables
        int beatInterval = 1; //interval representing one beat click = (60.0 / bpm) * sampleRate
        int samplesProcessed = 1; // samples processed before beat = totalSamples % interval
        int oneflag = numerator;  //oneflag signals a first beat of bar when oneflag = numerator, +=1 every main beat, reset to 1 after a bar

        //subdivision logic variables
        int subInterval = 0; //subInterval is beatInterval/subdivisions 
        int subSamplesProcessed = 0; /// samples processed before subbeat= totalSamples % subInterval;
        int beatCount = subdivisions; //beatCount keeps count of which subdivision we're on, +=1 when subdivision click is played, reset to 1 when main beat is finished



        //apvts of caller that created this instance of metronome
        juce::AudioProcessorValueTreeState* apvts;

        //file processing stuff
        juce::AudioFormatManager formatManager;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotHigh = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotLow = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotSub = nullptr;

        //===========================================================================================
        //TODO code for loading multiple files?
        //static const int numWAVs = 2;
        //std::array <std::unique_ptr <juce::AudioFormatReaderSource>, numWAVs> pMetronomeSamples;

       // void Metronome::audioEvent(juce::AudioBuffer<float>& buffer, Event eventType);




    private:
     

};