/*(
  ==============================================================================

    This file contains various utility classes and constants which are used 
    throughout the PolyGnome.

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
#include <string>
#include <JuceHeader.h>

using namespace std;

//TODO: make MIDI_STANDARD_OFFSET customizable by user in case their DAW uses a different MIDI standard
//misc constants
const int MIDI_STANDARD_OFFSET = 12;
const int MIN_MIDI_VALUE = 12;
const int MAX_MIDI_VALUE = 127 - MIDI_STANDARD_OFFSET;
const int MIDI_CHANNEL = 1;

//menu constants
const int MAX_TRACKS = 12;
const int MAX_BARS = 8;
const float MAX_BPM = 480;

//Machine constants
const int MAX_SUBDIVISIONS = 16;
const int MAX_VELOCITY = 128;
const int DEFAULT_VELOCITY = 100;
const int DEFAULT_MIDI_VALUE = 24; //0 - (128 - MAX_TRACKS)
const int DEFAULT_SUBDIVISIONS = 4; //1 -  MAX_SUBDIVISIONS
const double DEFAULT_SUSTAIN = 50.00; // 0 - 100

//UI size constants
const int PLUGIN_WIDTH = 1000;
const int PLUGIN_HEIGHT = 700;
const int MENU_WIDTH = 100;
const int BOTTOM_HEIGHT = 200;

//colour constants
const juce::Colour BACKGROUND_COLOUR = juce::Colours::black;
const juce::Colour REMINDER_COLOUR = juce::Colours::white;
const juce::Colour MAIN_COLOUR = juce::Colours::indigo;//default purple
const juce::Colour SECONDARY_COLOUR = juce::Colours::steelblue; //default blue
const juce::Colour ACCENT_COLOUR = juce::Colours::orange;//default orange
const juce::Colour DISABLED_COLOUR = juce::Colours::grey;
const juce::Colour DISABLED_DARK_COLOUR = juce::Colours::darkgrey; 
const juce::Colour ENABLED_COLOUR = juce::Colours::green; //default green

//reminder texts
const string BEAT_BUTTON_REMINDER = "Toggle Beat";
const string MUTE_BUTTON_REMINDER = "mute all beats for this track";
const string SUBDIVISION_SLIDER_REMINDER = "change the number of subdivisions per bar for this track";
const string MIDI_SLIDER_REMINDER = "change the MIDI value";
const string MIDI_TEXTEDITOR_REMINDER = "enter a valid MIDI string: <NOTE LETTER><optional #><-2 - 9> or integer: 0-127";
const string VELOCITY_SLIDER_REMINDER = "change velocity 0-128";
const string SUSTAIN_SLIDER_REMINDER = "change sustain 0-100%";
const string PLAY_BUTTON_REMINDER = "start/stop polygnome";
const string LOAD_PRESET_BUTTON_REMINDER = "load a .pgnome file";
const string SAVE_PRESET_BUTTON_REMINDER = "save a .pgnome file";
const string BPM_SLIDER_REMINDER = "change bpm";
const string RACK_SLIDER_REMINDER = "adjust how many bars per loop";
const string BAR_SELECT_BUTTON_REMINDER = "select a bar of the loop";
const string AUTO_LOOP_REMINDER = "automatically switch to page cooresponding to currently playing bar";
const string BAR_COPY_BUTTON_REMINDER = "copy currently selected page to different bar";


string midiIntToString(int midiValue);
int midiStringToInt(string midiValue);

juce::String getBeatToggleString(int barNum, int trackNum, int beatNum);
juce::String getSubdivisionsString(int barNum, int trackNum);
juce::String getVelocityString(int barNum, int trackNum);
juce::String getMidiValueString(int barNum, int trackNum);
juce::String getSustainString(int barNum, int trackNum);
juce::String getTrackEnableString(int barNum, int trackNum);

/*
class CustomComponent : public virtual juce::Component {
public:
    void mouseEnter(const juce::MouseEvent& event) override {
        isHoveredOver = true;
    }
    void mouseExit(const juce::MouseEvent& event) override {
        isHoveredOver = false;
    }
    bool isHoveredOver = false;
};
*/

class CustomSlider : public  juce::Slider {
    public:
        void mouseEnter(const juce::MouseEvent& event) override {
            isHoveredOver = true;
        }
        void mouseExit(const juce::MouseEvent& event) override {
            isHoveredOver = false;
        }
        bool isHoveredOver = false;
    private:
};

class CustomTextEditor : public  juce::TextEditor {
    public:
        void mouseEnter(const juce::MouseEvent& event) override {
            isHoveredOver = true;
        }
        void mouseExit(const juce::MouseEvent& event) override {
            isHoveredOver = false;
        }
        void focusGained(FocusChangeType cause) override {
            isFocussed = true;
        }
        void focusLost(FocusChangeType cause) override {
            isFocussed = false;
        }

        bool isFocussed = false;
        bool isHoveredOver = false;
    private:
};

class CustomTextButton : public  juce::TextButton {
    public:
        void mouseEnter(const juce::MouseEvent& event) override {
            isHoveredOver = true;
        }
        void mouseExit(const juce::MouseEvent& event) override {
            isHoveredOver = false;
        }

        void mouseUp(const juce::MouseEvent& event) override
        {
 
            if (event.mods.isRightButtonDown()) {
                DBG("TRUE");

                return;
            }
            else if (event.mods.isLeftButtonDown()) {
                triggerClick();
            }
        }
        bool isHoveredOver = false;
    private:    
};

class CustomToggleButton : public juce::ToggleButton {
    public:
        void mouseEnter(const juce::MouseEvent& event) override {
            isHoveredOver = true;
        }
        void mouseExit(const juce::MouseEvent& event) override {
            isHoveredOver = false;
        }
        bool isHoveredOver = false;
    private:
};



