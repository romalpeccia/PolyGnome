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
    void paintPolyRhythmMachine(juce::Graphics& g);
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


    //polytrack machine buttons
    juce::ToggleButton muteButtons[MAX_MIDI_CHANNELS];
    juce::TextButton beatButtons[MAX_MIDI_CHANNELS][MAX_TRACK_LENGTH];
    juce::Slider subdivisionSliders[MAX_MIDI_CHANNELS];
    juce::Slider midiSliders[MAX_MIDI_CHANNELS];
    juce::TextEditor midiTextEditors[MAX_MIDI_CHANNELS];
    juce::Slider velocitySliders[MAX_MIDI_CHANNELS];

    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> muteButtonAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> beatButtonAttachments[MAX_MIDI_CHANNELS][MAX_TRACK_LENGTH];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subdivisionSliderAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midiSliderAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocitySliderAttachments[MAX_MIDI_CHANNELS];

    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();
    std::vector<juce::Component*> getTrackComps(int index);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyGnomeAudioProcessorEditor)
};


