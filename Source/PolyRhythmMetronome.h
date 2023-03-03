/*
  ==============================================================================

    PolyRhythmMetronome.h
    Created: 6 Jan 2022 3:59:04pm
    Author:  Romal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

using namespace std;
//==============================================================================
/*
*/
class PolyRhythmMetronome  : public juce::Component
{
public:
    PolyRhythmMetronome();
    PolyRhythmMetronome(juce::AudioProcessorValueTreeState* _apvts);
    ~PolyRhythmMetronome() override;

    void prepareToPlay(double _sampleRate, int samplesPerBlock);
    void getNextAudioBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer);// override; //no override?
    void resetAll() ;
    void resetParams();
    int getRhythm1Counter() { return rhythm1Counter; }
    int getRhythm2Counter() { return rhythm2Counter; }
    int getTotalSamples() { return totalSamples; }





private:

    void PolyRhythmMetronome::handleNoteTrigger(juce::MidiBuffer&, int noteNumber);

    //TODO make value more descriptive... subdivisions?
    int rhythm1Value = 4; //represented as NUMERATOR in apvts
    int rhythm2Value = 1; //represented as SUBDIVISION in apvts
    double bpm = 60;

    //overall logic variables
    int totalSamples = 0; //total samples since start time
    double sampleRate = 0; //sampleRate from app, usually 44100

    //rhythm1 logic variables
    int rhythm1Interval = 0;
    int rhythm1SamplesProcessed = 0; // samples processed before beat = totalSamples % interval
    int rhythm1Counter = 0;
    //rhythm2 logic variables
    int rhythm2Interval = 0;
    int rhythm2SamplesProcessed = 0; /// samples processed before beat= totalSamples % rhythm2Interval;
    int rhythm2Counter = 0;

    //apvts of caller that created this instance of polyrhythmmetronome
    juce::AudioProcessorValueTreeState* apvts;

    //file processing stuff
    juce::AudioFormatManager formatManager;
    std::unique_ptr <juce::AudioFormatReaderSource> rimShotHigh = nullptr;
    std::unique_ptr <juce::AudioFormatReaderSource> rimShotLow = nullptr;
    std::unique_ptr <juce::AudioFormatReaderSource> rimShotSub = nullptr;

   const double startTime = juce::Time::getMillisecondCounterHiRes();

    //
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyRhythmMetronome)
};
