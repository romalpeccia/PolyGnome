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

    void colorSlider(juce::Slider &slider, juce::Colour thumbColour, juce::Colour textBoxTextColour, juce::Colour textBoxBackgroundColour, juce::Colour textBoxOutlineColour, bool trackEnabled);
    void colorTextEditor(juce::TextEditor &textEditor, juce::Colour textColour, juce::Colour focusedOutlineColour, juce::Colour outlineColour, juce::Colour backgroundColour, bool trackEnabled);
    //void colorTextButton(juce::TextButton& textButton, juce::Colour buttonColour, bool buttonEnabled, bool trackEnabled);
private:
    PolyGnomeAudioProcessor& audioProcessor;

    juce::Image logo;
    juce::TextButton playButton{ "Play" };
    juce::TextButton loadPresetButton{ "load preset" };
    juce::TextButton savePresetButton{ "save preset" };

    void loadPreset();
    void savePreset();
    std::unique_ptr<juce::FileChooser> fileChooser;


    RotarySliderWithLabels    bpmSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bpmAttachment;


    juce::ToggleButton muteButtons[MAX_MIDI_CHANNELS];
    juce::TextButton beatButtons[MAX_MIDI_CHANNELS][MAX_TRACK_LENGTH];
    juce::Slider subdivisionSliders[MAX_MIDI_CHANNELS];
    juce::Slider midiSliders[MAX_MIDI_CHANNELS];
    juce::TextEditor midiTextEditors[MAX_MIDI_CHANNELS];
    juce::Slider velocitySliders[MAX_MIDI_CHANNELS];
    juce::Slider sustainSliders[MAX_MIDI_CHANNELS];

    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> muteButtonAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> beatButtonAttachments[MAX_MIDI_CHANNELS][MAX_TRACK_LENGTH];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subdivisionSliderAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midiSliderAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocitySliderAttachments[MAX_MIDI_CHANNELS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainSliderAttachments[MAX_MIDI_CHANNELS];

    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();
    std::vector<juce::Component*> getTrackComps(int index);



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyGnomeAudioProcessorEditor)
};


