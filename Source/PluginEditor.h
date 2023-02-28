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
    void MetroGnomeAudioProcessorEditor::drawPolyrhythmVisual(juce::Graphics& g, int radius, int width, int height, int X, int Y, int rhythmValue, float radiusSkew, juce::Colour color1, juce::Colour color, int index);

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
    //the attachment attaches an APVTS param to a slider
    RotarySliderWithLabels    bpmSlider, subdivisionSlider, numeratorSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bpmAttachment, subdivisionAttachment, numeratorAttachment;
  

    juce::ToggleButton Rhythm1Buttons[MAX_LENGTH];
    juce::ToggleButton Rhythm2Buttons[MAX_LENGTH];
    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroGnomeAudioProcessorEditor)
};

