/*(
  ==============================================================================

    This file contains various utility functions which are used throughout
    the PolyGnome.

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

#include "Utilities.h"
#include <JuceHeader.h>


juce::String getBeatToggleString(int barNum, int trackNum, int beatNum) {
    return to_string(barNum)+ "_BEAT_" + to_string(trackNum) + "_" + to_string(beatNum) + "_TOGGLE";
}

juce::String getSubdivisionsString(int barNum, int trackNum) {
    return to_string(barNum) + "_SUBDIVISIONS_" + to_string(trackNum);
}
juce::String getVelocityString(int barNum, int trackNum) {
    return to_string(barNum) + "_VELOCITY_" + to_string(trackNum);
}
juce::String getMidiValueString(int barNum, int trackNum) {
    return to_string(barNum) + "_MIDI_VALUE" + to_string(trackNum);
}
juce::String getSustainString(int barNum, int trackNum) {
    return to_string(barNum) + "_SUSTAIN_" + to_string(trackNum);
}
juce::String getTrackEnableString(int barNum, int trackNum) {
    return to_string(barNum) + "_TRACK_" + to_string(trackNum) + "_ENABLE";
}


string midiIntToString(int midiValue) {
    //takes an integer and returns it's cooresponding MIDI string value
    if (midiValue > 127 || midiValue < 0)
    {
        return "";
    }
    string noteName;
    string noteInt;
    switch (midiValue % 12) {
    case 0:
        noteName = "C";
        break;
    case 1:
        noteName = "C#";
        break;
    case 2:
        noteName = "D";
        break;
    case 3:
        noteName = "D#";
        break;
    case 4:
        noteName = "E";
        break;
    case 5:
        noteName = "F";
        break;
    case 6:
        noteName = "F#";
        break;
    case 7:
        noteName = "G";
        break;
    case 8:
        noteName = "G#";
        break;
    case 9:
        noteName = "A";
        break;
    case 10:
        noteName = "A#";
        break;
    case 11:
        noteName = "B";
        break;
    }
    switch (midiValue / 12) {
    case 0:
        noteInt = "-1";
        break;
    case 1:
        noteInt = "0";
        break;
    case 2:
        noteInt = "1";
        break;
    case 3:
        noteInt = "2";
        break;
    case 4:
        noteInt = "3";
        break;
    case 5:
        noteInt = "4";
        break;
    case 6:
        noteInt = "5";
        break;
    case 7:
        noteInt = "6";
        break;
    case 8:
        noteInt = "7";
        break;
    case 9:
        noteInt = "8";
        break;
    case 10:
        noteInt = "9";
        break;
    case 11:
        noteInt = "10";
        break;
    }

    string midiString = noteName.append(noteInt);
    return midiString;
}


int midiStringToInt(string midiValue) {
    //takes an MIDI string and returns it's cooresponding MIDI integer value
    //a value MIDI string can be some combination of (Note char)(optional sharp)(optional negative symbol)(note int), giving us 4 possible ways to parse a midi string

    string noteName;
    string noteValue;
    int noteMultiplier;
    int noteAddition;

    if (midiValue[0] != '\0' && midiValue[1] != '\0') {
        noteName = midiValue[0];
        if (midiValue[1] == '#') {
            noteName.append("#");
            if (midiValue[2] != '\0') {
                noteValue.append(&midiValue[2]);
            }
            else return -1;
        }
        else {
            noteValue.append(&midiValue[1]);
        }
    }
    else return -1;


    if (noteName == "C" || noteName == "c") {
        noteAddition = 0;
    }
    else if (noteName == "C#" || noteName == "c#") {
        noteAddition = 1;
    }
    else if (noteName == "D" || noteName == "d") {
        noteAddition = 2;
    }
    else if (noteName == "D#" || noteName == "d#") {
        noteAddition = 3;
    }
    else if (noteName == "E" || noteName == "e") {
        noteAddition = 4;
    }
    else if (noteName == "F" || noteName == "f") {
        noteAddition = 5;
    }
    else if (noteName == "F#" || noteName == "f#") {
        noteAddition = 6;
    }
    else if (noteName == "G" || noteName == "g") {
        noteAddition = 7;
    }
    else if (noteName == "G#" || noteName == "g#") {
        noteAddition = 8;
    }
    else if (noteName == "A" || noteName == "a") {
        noteAddition = 9;
    }
    else if (noteName == "A#" || noteName == "a#") {
        noteAddition = 10;
    }
    else if (noteName == "B" || noteName == "b") {
        noteAddition = 11;
    }
    else {
        return -1;
    }

    if (noteValue == "-1") {
        noteMultiplier = 0;
    }
    else if (noteValue == "0") {
        noteMultiplier = 1;
    }
    else if (noteValue == "1") {
        noteMultiplier = 2;
    }
    else if (noteValue == "2") {
        noteMultiplier = 3;
    }
    else if (noteValue == "3") {
        noteMultiplier = 4;
    }
    else if (noteValue == "4") {
        noteMultiplier = 5;
    }
    else if (noteValue == "5") {
        noteMultiplier = 6;
    }
    else if (noteValue == "6") {
        noteMultiplier = 7;
    }
    else if (noteValue == "7") {
        noteMultiplier = 8;
    }
    else if (noteValue == "8") {
        noteMultiplier = 9;
    }
    else if (noteValue == "9") {
        noteMultiplier = 10;
    }
    else {
        return -1;
    }
    int result = 12 * noteMultiplier + noteAddition;
    if (result > 127 || result < 0) {
        return -1;
    }
    else {
        return result;
    }

}