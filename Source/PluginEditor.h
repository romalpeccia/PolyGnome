/*(
  ==============================================================================

    This file handles the creation and painting of UI components of the PolyGnome.

  ==============================================================================
    Copyright(C) 2024 Romal Peccia

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < https://www.gnu.org/licenses/>.
*/
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Utilities.h"
#include "MenuComponent.h"
#include "TrackComponent.h"
#include "CustomKeyboardComponent.h"


class BarComponent {
    public:
        TrackComponent tracks[MAX_TRACKS];
    private:
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

        juce::Rectangle<int> getTrackArea();

        void resized() override;
        void timerCallback() override {
            repaint();
        };

        //play state functions TODO: (potentially obsolete in some cases, since menu.playButton was removed. keep it hidden for now in case it is helpful for standalone mode)
        void toggleAudioProcessorChildrenStates();
        void togglePlayState();
        void togglePlayStateOff();
        void togglePlayStateOn();

        juce::String getCurrentMouseOverText();

        void colorSlider(juce::Slider &slider, juce::Colour thumbColour, juce::Colour textBoxTextColour, juce::Colour textBoxBackgroundColour, juce::Colour textBoxOutlineColour, bool trackEnabled);
        void colorTextEditor(juce::TextEditor &textEditor, juce::Colour textColour, juce::Colour focusedOutlineColour, juce::Colour outlineColour, juce::Colour backgroundColour, bool trackEnabled);
        //void colorTextButton(juce::TextButton& textButton, juce::Colour buttonColour, bool buttonEnabled, bool trackEnabled);

        //component initializations 
        void initializeMenuComponents();
        void initializeMachineComponents();
        void loadPreset();
        void savePreset();
    private:
        PolyGnomeAudioProcessor& audioProcessor;
    
        std::vector<juce::Component*> getVisibleComps();
        std::vector<juce::Component*> getHiddenComps();
        std::vector<juce::Component*> getAllComps();
        std::vector<juce::Component*> getTrackComps(int barIndex, int trackIndex);

        std::unique_ptr<juce::FileChooser> fileChooser;

        juce::Image logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);
        juce::Image keyboardIcon = juce::ImageCache::getFromMemory(BinaryData::keyboard_png, BinaryData::keyboard_pngSize);
        juce::Image trackIcon = juce::ImageCache::getFromMemory(BinaryData::track_png, BinaryData::track_pngSize);
        juce::Image sustainIcon = juce::ImageCache::getFromMemory(BinaryData::sustain_png, BinaryData::sustain_pngSize);
        juce::Image velocityIcon = juce::ImageCache::getFromMemory(BinaryData::velocity_png, BinaryData::velocity_pngSize);
        juce::Image enableIcon = juce::ImageCache::getFromMemory(BinaryData::enable_png, BinaryData::enable_pngSize);

        MenuComponent menu;
        BarComponent bars[MAX_BARS];

        CustomKeyboardComponent keyboard;
        beatID selectedBeatID; // ID of the beat that was most recently clicked
        beatID prevBeatID;    // ID of the beat that was clicked before the most recent click
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyGnomeAudioProcessorEditor)
};



