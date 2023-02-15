/*
  ==============================================================================

    PolyRhythmMetronome.h
    Created: 6 Jan 2022 3:59:04pm
    Author:  Romal

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Metronome.h"
using namespace std;
//==============================================================================
/*
*/
class PolyRhythmMetronome  : public juce::Component, public Metronome
{
public:
    PolyRhythmMetronome();
    PolyRhythmMetronome(juce::AudioProcessorValueTreeState* _apvts);
    ~PolyRhythmMetronome() override;

    void getNextAudioBlock(juce::AudioBuffer<float>& buffer);// override; //no override?
    void resetall() ;
    void resetparams();
    int getRhythm1Counter() { return rhythm1Counter; }
    int getRhythm2Counter() { return rhythm2Counter; }
    int getTotalSamples() { return totalSamples; }
private:
    // inherited numerator controls rhythm1
    // inherited subdivisions controls rhythm2
    int rhythm1Counter = 0;
    int rhythm2Counter = 0;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyRhythmMetronome)
};
