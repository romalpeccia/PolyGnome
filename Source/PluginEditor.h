/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "Utilities.h"

//==============================================================================
/**
*/
class PolyGnomeAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PolyGnomeAudioProcessorEditor(PolyGnomeAudioProcessor&);
    ~PolyGnomeAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void paintPolyRhythmMachineMode(juce::Graphics& g);
    void changeMenuButtonColors(juce::TextButton *buttonOn);

    juce::Rectangle<int> getVisualArea();

    void resized() override;
    void timerCallback() override {
        repaint();
    };

    void toggleAudioProcessorChildrenStates();
    void togglePlayState();
    void togglePlayStateOff();
    void togglePlayStateOn();

private:
    PolyGnomeAudioProcessor& audioProcessor;

    //persistent comps
    juce::Image logo;
    juce::TextButton playButton{ "Play" };
    juce::TextButton loadPresetButton{ "load preset" };
    juce::TextButton savePresetButton{ "save preset" };

    void loadPreset();
    void savePreset();
    std::unique_ptr<juce::FileChooser> fileChooser;

    //Sliders
    //the attachment attaches an APVTS param to a slider
    RotarySliderWithLabels    bpmSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bpmAttachment;


    //polyrhythm machine buttons
    juce::TextButton polyRhythmMachineButtons[MAX_MIDI_CHANNELS][MAX_LENGTH];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> polyRhythmMachineButtonAttachments[MAX_MIDI_CHANNELS][MAX_LENGTH];
    juce::Slider polyRhythmMachineSubdivisionSliders[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyRhythmMachineSubdivisionSliderAttachments[MAX_MIDI_CHANNELS];
    juce::Slider polyRhythmMachineMidiSliders[MAX_MIDI_CHANNELS];
    juce::TextEditor polyRhythmMachineMidiTextEditors[MAX_MIDI_CHANNELS];
    juce::ToggleButton polyRhythmMachineMuteButtons[MAX_MIDI_CHANNELS];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> polyRhythmMachineMuteButtonAttachments[MAX_MIDI_CHANNELS];

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> polyRhythmMachineMidiSliderAttachments[MAX_MIDI_CHANNELS];


    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyGnomeAudioProcessorEditor)
};


