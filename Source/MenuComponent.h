/*
  ==============================================================================

    MenuComponent.h
    Created: 25 Aug 2024 4:34:25pm
    Author:  romal

  ==============================================================================
*/

#pragma once
#include "Utilities.h"
#include <JuceHeader.h>

class MenuComponent : public virtual juce::Component { 

    public:
        MenuComponent() {
            loadPresetButton.setButtonText("Load Preset");
            loadPresetButton.setHelpText(LOAD_PRESET_BUTTON_REMINDER);

            savePresetButton.setButtonText("Save Preset");
            savePresetButton.setHelpText(SAVE_PRESET_BUTTON_REMINDER);

            barSlider.setHelpText(BAR_SLIDER_REMINDER);
            barSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, 25);
            
            reminderTextEditor.setMultiLine(true);

            autoLoopButton.setHelpText(AUTO_LOOP_REMINDER);
            autoLoopButton.setButtonText("Auto-Loop");

            for (int k = 0; k < MAX_BARS; k++) {
                barCopyButtons[k].setHelpText(BAR_COPY_BUTTON_REMINDER);
                barCopyButtons[k].setButtonText(to_string(k + 1));

                barSelectButtons[k].setHelpText(BAR_SELECT_BUTTON_REMINDER);
                barSelectButtons[k].setButtonText(to_string(k + 1));
            }
        };

        CustomTextButton playButton;
        CustomTextButton loadPresetButton;
        CustomTextButton savePresetButton;
        CustomTextEditor reminderTextEditor;
        CustomTextButton barSelectButtons[MAX_BARS];
        CustomTextButton barCopyButtons[MAX_BARS];
        CustomSlider barSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> barSliderAttachment;
        CustomTextButton autoLoopButton;
    private:
};