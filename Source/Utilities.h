/*
  ==============================================================================

    Constants.h
    Created: 28 Feb 2023 12:50:03pm
    Author:  romal

  ==============================================================================
*/
#pragma once
#include <string>
#include <JuceHeader.h>

using namespace std;


const int MAX_TRACK_LENGTH = 16;
const int MAX_MIDI_CHANNELS = 12;
const int MAX_RACKS = 8;

const float MAX_BPM = 480;
const int DEFAULT_VELOCITY = 100;
const int DEFAULT_MIDI_VALUE = 36; //0 - (128 - MAX_MIDI_CHANNELS)
const int DEFAULT_SUBDIVISIONS = 4; //1 -  MAX_TRACK_LENGTH
const int DEFAULT_SUSTAIN = 50.00; // 0 - 100

const juce::Colour BACKGROUND_COLOUR = juce::Colours::black;
const juce::Colour REMINDER_COLOUR = juce::Colours::white;
const juce::Colour MAIN_COLOUR = juce::Colours::indigo;//default purple
const juce::Colour SECONDARY_COLOUR = juce::Colours::steelblue; //default blue
const juce::Colour ACCENT_COLOUR = juce::Colours::orange;//default orange
const juce::Colour DISABLED_COLOUR = juce::Colours::grey;
const juce::Colour DISABLED_DARK_COLOUR = juce::Colours::darkgrey; 
const juce::Colour ENABLED_COLOUR = juce::Colours::green; //default green

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
const string RACK_BUTTON_REMINDER = "select a bar of the loop";

string midiIntToString(int midiValue);
int midiStringToInt(string midiValue);

juce::String getBeatToggleString(int i, int j);
juce::String getSubdivisionsString(int i);
juce::String getVelocityString(int i);
juce::String getMidiValueString(int i);
juce::String getSustainString(int i);
juce::String getTrackEnableString(int i);

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



