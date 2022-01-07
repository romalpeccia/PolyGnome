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
    //note: inherited variables from Metronome class are being used for other purposes, perhaps refactor them later on
    // subdivisions represent the 2nd rhythm for the purposes of this class
    int rhythm1Counter = 0;
    int rhythm2Counter = 0;

    float clock1 = 0;
    float clock2 = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyRhythmMetronome)
};
