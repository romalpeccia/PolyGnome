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


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MetroGnomeAudioProcessor& audioProcessor;
    juce::TextButton playButton{ "Play" };

    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    juce::Slider bpmSlider, subdivisionSlider, numeratorSlider;
    //juce::ComboBox timeSignatureBox;

   // Attachment bpmAttachment, subdivisionAttachment, numeratorAttachment;

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroGnomeAudioProcessorEditor)
};

