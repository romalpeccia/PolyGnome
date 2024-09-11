/*
  ==============================================================================

    TrackComponent.h
    Created: 25 Aug 2024 4:53:29pm
    Author:  romal

  ==============================================================================
*/

#pragma once

#include "Utilities.h"
#include <JuceHeader.h>

class TrackComponent {
    public:
        TrackComponent() {

            muteButton.setClickingTogglesState(true);
            muteButton.setHelpText(MUTE_BUTTON_REMINDER);

            subdivisionSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            subdivisionSlider.setHelpText(SUBDIVISION_SLIDER_REMINDER);

            velocitySlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            velocitySlider.setHelpText(VELOCITY_SLIDER_REMINDER);

            sustainSlider.setHelpText(SUSTAIN_SLIDER_REMINDER);

            midiSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            midiSlider.setHelpText(MIDI_SLIDER_REMINDER);

            midiTextEditor.setReadOnly(false);

            midiTextEditor.setHelpText(MIDI_TEXTEDITOR_REMINDER);
            for (int j = 0; j < MAX_SUBDIVISIONS; j++) {
                beatButtons[j].setClickingTogglesState(true);
                beatButtons[j].setHelpText(BEAT_BUTTON_REMINDER);

                beatMidiSliders[j].setHelpText(BEAT_MIDI_REMINDER);
            }


        }
        //MIDDLE COMPONENTS (individual beat controls)
        BeatButton beatButtons[MAX_SUBDIVISIONS];
        CustomSlider beatMidiSliders[MAX_SUBDIVISIONS];
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> beatMidiSliderAttachments[MAX_SUBDIVISIONS];
        juce::Label beatLabels[MAX_SUBDIVISIONS];

        //RIGHT COMPONENTS (composition controls)
        CustomSlider subdivisionSlider;
        CustomSlider midiSlider;
        CustomTextEditor midiTextEditor;

        //LEFT COMPONENTS (extra controls)
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

    private:
};