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
    void paintDefaultMode(juce::Graphics&);
    void paintPolyRMode(juce::Graphics&);


    juce::Rectangle<int> getVisualArea();

    void resized() override;
    void timerCallback() override {
        repaint();
    };

    void play();


private:
    MetroGnomeAudioProcessor& audioProcessor;
    //TimeVisualizerComponent timeVisualizer;

    //persistent comps
    juce::Image logo;
    juce::TextButton playButton{ "Start" };
    juce::TextButton defaultModeButton{ "Default" };
    juce::TextButton polyRModeButton{ "PolyRhythm" };
    juce::TextButton polyMModeButton{ "PolyMeter" };
    juce::TextButton placeholderButton{ "Placeholder" };

    //Sliders
    RotarySliderWithLabels    bpmSlider, subdivisionSlider, numeratorSlider;
    //juce::ComboBox timeSignatureBox;
    juce::AudioProcessorValueTreeState::SliderAttachment bpmAttachment, subdivisionAttachment, numeratorAttachment;

    std::vector<juce::Component*> getComps();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroGnomeAudioProcessorEditor)
};

