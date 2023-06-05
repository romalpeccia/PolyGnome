/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Utilities.h"


class TrackComponent {
public:
    //MIDDLE COMPONENTS
    CustomTextButton beatButtons[MAX_SUBDIVISIONS];

    //RIGHT COMPONENTS
    CustomSlider subdivisionSlider;
    CustomSlider midiSlider;
    CustomTextEditor midiTextEditor;

    //LEFT COMPONENTS
    CustomToggleButton muteButton;
    CustomSlider velocitySlider;
    CustomSlider sustainSlider;

    //ATTACHMENTS
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> muteButtonAttachment;
    std::unique_ptr <juce::AudioProcessorValueTreeState::ButtonAttachment> beatButtonAttachments[MAX_SUBDIVISIONS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subdivisionSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midiSliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocitySliderAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainSliderAttachment;

};


class BarComponent {
public:
    TrackComponent tracks[MAX_TRACKS];

};


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

    juce::Rectangle<int> getVisualArea();

    void resized() override;
    void timerCallback() override {
        repaint();
    };

    void toggleAudioProcessorChildrenStates();
    void togglePlayState();
    void togglePlayStateOff();
    void togglePlayStateOn();
    juce::String getCurrentMouseOverText();

    void colorSlider(juce::Slider &slider, juce::Colour thumbColour, juce::Colour textBoxTextColour, juce::Colour textBoxBackgroundColour, juce::Colour textBoxOutlineColour, bool trackEnabled);
    void colorTextEditor(juce::TextEditor &textEditor, juce::Colour textColour, juce::Colour focusedOutlineColour, juce::Colour outlineColour, juce::Colour backgroundColour, bool trackEnabled);
    //void colorTextButton(juce::TextButton& textButton, juce::Colour buttonColour, bool buttonEnabled, bool trackEnabled);





private:
    PolyGnomeAudioProcessor& audioProcessor;

    std::vector<juce::Component*> getVisibleComps();
    std::vector<juce::Component*> getHiddenComps();
    std::vector<juce::Component*> getAllComps();
    std::vector<juce::Component*> getTrackComps(int barIndex, int trackIndex);

    void loadPreset();
    void savePreset();
    std::unique_ptr<juce::FileChooser> fileChooser;




    // TOP LEFT Components
    juce::Image logo;
    CustomTextButton playButton;
    CustomTextButton loadPresetButton;
    CustomTextButton savePresetButton;
    CustomTextEditor reminderTextEditor;
    CustomTextButton barSelectButtons[MAX_BARS];
    CustomTextButton barCopyButtons[MAX_BARS];
    CustomSlider barSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> barSliderAttachment;

    BarComponent bars[MAX_BARS];

    CustomTextButton autoLoopButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyGnomeAudioProcessorEditor)
};



