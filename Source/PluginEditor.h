/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

//==============================================================================
/**
*/
class MetroGnomeAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    MetroGnomeAudioProcessorEditor (MetroGnomeAudioProcessor&);
    ~MetroGnomeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override {
        repaint();
    };

    void play();


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MetroGnomeAudioProcessor& audioProcessor;
    //TimeVisualizerComponent timeVisualizer;




    juce::Image logo;


    juce::TextButton playButton{ "Start" };


    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    //juce::Slider 
    RotarySliderWithLabels    bpmSlider, subdivisionSlider, numeratorSlider;
    //juce::ComboBox timeSignatureBox;

    Attachment bpmAttachment, subdivisionAttachment, numeratorAttachment;

    std::vector<juce::Component*> getComps();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroGnomeAudioProcessorEditor)
};

