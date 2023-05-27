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
#include "LookAndFeel.h"

using std::string;

const int MAX_TRACK_LENGTH = 16;
const int MAX_MIDI_CHANNELS = 12;
const int DEFAULT_VELOCITY = 100;
const int DEFAULT_MIDI_VALUE = 36; //should never be greater than (128 - MAX_MIDI_CHANNELS)
const int DEFAULT_SUBDIVISIONS = 4; //should never be greater than MAX_TRACK_LENGTH

const juce::Colour BACKGROUND_COLOUR = juce::Colours::black;
const juce::Colour MAIN_COLOUR = juce::Colours::indigo;//default purple
const juce::Colour SECONDARY_COLOUR = juce::Colours::steelblue; //default blue
const juce::Colour ACCENT_COLOUR = juce::Colours::orange;//default orange
const juce::Colour DISABLED_COLOUR = juce::Colours::grey;
const juce::Colour DISABLED_DARK_COLOUR = juce::Colours::darkgrey; 
const juce::Colour ENABLED_COLOUR = juce::Colours::green; //default green

string midiIntToString(int midiValue);
int midiStringToInt(string midiValue);