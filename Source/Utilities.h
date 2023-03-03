/*
  ==============================================================================

    Constants.h
    Created: 28 Feb 2023 12:50:03pm
    Author:  romal

  ==============================================================================
*/
#pragma once
#include <string>
using std::string;

const int MAX_LENGTH = 12;
const int MAX_MIDI_CHANNELS = 12;
string midiIntToString(int midiValue);
int midiStringToInt(string midiValue);