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
const string BEAT_BUTTON_REMINDER = "Toggle Beat ON/OFF";
const string MUTE_BUTTON_REMINDER = "Mute all beats for this track";
const string SUBDIVISION_SLIDER_REMINDER = "Change the number of subdivisions per bar for this track";
const string MIDI_SLIDER_REMINDER = "Change the MIDI value for all beats on this track";
const string MIDI_TEXTEDITOR_REMINDER = "change all MIDI values for all beats on this track. Use your MIDI instrument or enter a valid MIDI string: <NOTE LETTER><optional #><-2 - 9> or integer: 0-127";
const string VELOCITY_SLIDER_REMINDER = "Change MIDI Velocity from 0-128";
const string SUSTAIN_SLIDER_REMINDER = "Change sustain (MIDI note length) from 0-100%";
const string PLAY_BUTTON_REMINDER = "start/stop polygnome";
const string LOAD_PRESET_BUTTON_REMINDER = "Load a .pgnome file";
const string SAVE_PRESET_BUTTON_REMINDER = "Save a .pgnome file";
const string BPM_SLIDER_REMINDER = "Change BPM";
const string BAR_SLIDER_REMINDER = "Adjust how many bars per loop";
const string BAR_SELECT_BUTTON_REMINDER = "Select a bar of the loop";
const string AUTO_LOOP_REMINDER = "Automatically show the next bar when the currently selected bar finishes playing";
const string BAR_COPY_BUTTON_REMINDER = "Copy the currently selected page to a different bar";
const string BEAT_MIDI_REMINDER = "Change the MIDI value for the currently highlighted beat using the slider or your MIDI instrument";

//conversion from MIDI strings like C#4 to integers like 37
string midiIntToString(int midiValue);
int midiStringToInt(string midiValue);

//helper functions so I don't have to repeat giant strings in apvts.getRawParameterValue calls
juce::String getBeatToggleString(int barNum, int trackNum, int beatNum);
juce::String getBeatMidiString(int barNum, int trackNum, int beatNum);
juce::String getSubdivisionsString(int barNum, int trackNum);
juce::String getVelocityString(int barNum, int trackNum);
juce::String getMidiValueString(int barNum, int trackNum);
juce::String getSustainString(int barNum, int trackNum);
juce::String getTrackEnableString(int barNum, int trackNum);

class APVTSComponent {
    //any component that needs the Processor or the APVTS to function
public:
    APVTSComponent();
    APVTSComponent(juce::AudioProcessorValueTreeState* _apvts) {
        apvts = _apvts;
    };
    //apvts of caller that created this component
    juce::AudioProcessorValueTreeState* apvts;

};

// Base class with common mouseEnter and mouseExit functionality
class HoverableComponent : public juce::Component {
public:
    void mouseEnter(const juce::MouseEvent& event) override {
        isHoveredOver = true;
    }

    void mouseExit(const juce::MouseEvent& event) override {
        isHoveredOver = false;
    }

protected:
    bool isHoveredOver = false;
};

// CustomSlider class, now inherits from HoverableComponent
class CustomSlider : public HoverableComponent, public juce::Slider {
public:
    CustomSlider();
    ~CustomSlider();

private:
};

// CustomTextEditor class, now inherits from HoverableComponent
class CustomTextEditor : public HoverableComponent, public juce::TextEditor {
public:
    CustomTextEditor();

    void focusGained(FocusChangeType cause) override {
        isFocused = true;
    }

    void focusLost(FocusChangeType cause) override {
        isFocused = false;
    }

private:
    bool isFocused = false;
};

// CustomTextButton class, now inherits from HoverableComponent
class CustomTextButton : public HoverableComponent, public juce::TextButton {
public:
    CustomTextButton();

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





class BeatMenu : public juce::Component {

    public:
        BeatMenu() {
            //setSize(400, 400);

            /*
            flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
            flexBox.items.add(juce::FlexItem(300, 100, beatMidiSlider));
            flexBox.items.add(juce::FlexItem(300, 100, arpButton));
            flexBox.performLayout(getBounds());
            */
        }
        ~BeatMenu() {

        }

    
        CustomSlider beatMidiSlider;
        juce::Label label;
        CustomTextButton arpButton;
    private:
        /*
        midi slider
            Arp toggle
            Num arps
            Speed
                Type
            Root note
            Major minor toggle
        */



        juce::FlexBox flexBox;
};

class beatID {
    public:
        beatID() {
            _barID = 0;
            _trackID = 0;
            _beatID = 0;
        }
        beatID(int k, int i, int j) {
            _barID = k;
            _trackID = i;
            _beatID = j;
        }
        void setbeatID(int k, int i, int j) {
            _barID = k;
            _trackID = i;
            _beatID = j;
        };
        int _barID;
        int _trackID;
        int _beatID;
        //bool operator==(beatID const&)  ;

};

class BeatButton : public  juce::TextButton {
    public:
        //commented out code is for potential CallOutBox implementation: need a way to keep the components from getting deleted in memory when the calloutBox closes.
        BeatButton() {
            apvts = NULL;
            selectedBeatPtr = NULL;
        }
        void mouseEnter(const juce::MouseEvent& event) override {
            isHoveredOver = true;
        }
        void mouseExit(const juce::MouseEvent& event) override {
            isHoveredOver = false;
        }

        void mouseUp(const juce::MouseEvent& event) override
        {
            selectedBeatPtr->setbeatID(beatId._barID, beatId._trackID, beatId._beatID);
            if (event.mods.isRightButtonDown()) {
  
            }
            else if (event.mods.isLeftButtonDown()) {
                triggerClick();
            }
        }
        bool isHoveredOver = false;


        beatID beatId;
        juce::AudioProcessorValueTreeState* apvts;
        beatID* selectedBeatPtr;
    private:
};



//notes on adding new components
/*
add helper text in utilities 
component and attachment declaration in cooresponding parent class in editor
add param to apvts in juce::AudioProcessorValueTreeState::ParameterLayout PolyGnomeAudioProcessor::createParameterLayout()
set helptext/style in editor
set helptext polling in getCurrentMouseOverText in pluginEditor.cpp
add component to getHiddenComps or getVisibleComps in pluginEditor.cpp
paint component in paint() in pluginEditor.cpp
*/