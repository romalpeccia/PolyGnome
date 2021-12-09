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
        int getbeatflag() {return beatflag;}

    private:


        //User params
        int numerator = 4; //numerator of time signature
        int subdivisions = 1; // amount of subdivisions, 1 = turns off subdivision logic 
        double bpm = 60;

        //metronome logic variables
        int totalSamples = 0; //total samples since start time
        int samplesProcessed = 0; // samples processed before beat = totalSamples % interval
        int beatInterval = 0; //interval representing one beat click = (60.0 / bpm) * sampleRate
        double sampleRate = 0; 


        //first beat and subdivision logic variables
        int subInterval = 0; //subInterval is beatInterval/subdivisions 
        int oneflag = numerator;  //oneflag signals a first beat of bar when oneflag = numerator, +=1 every main beat, reset to 1 after a bar
        int beatflag = subdivisions; //beatflag keeps count of which subdivision we're on, +=1 when subdivision click is played, reset to 1 when main beat is finished


        //apvts of pluginprocessor that created this instance of metronome
        juce::AudioProcessorValueTreeState* apvts;

        //file processing stuff
        juce::AudioFormatManager formatManager;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotHigh = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotLow = nullptr;
        std::unique_ptr <juce::AudioFormatReaderSource> rimShotSub = nullptr;
        //TODO code for loading multiple files?
        //static const int numWAVs = 2;
        //std::array <std::unique_ptr <juce::AudioFormatReaderSource>, numWAVs> pMetronomeSamples;




};