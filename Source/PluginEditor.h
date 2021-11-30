/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class MetroGnomeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MetroGnomeAudioProcessorEditor (MetroGnomeAudioProcessor&);
    ~MetroGnomeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    void play();
    void stop();

    enum class PlayState
    {
        playing,
        stopped
    };

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MetroGnomeAudioProcessor& audioProcessor;
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroGnomeAudioProcessorEditor)
};

