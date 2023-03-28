/*
  ==============================================================================

    Utilities.cpp
    Created: 3 Mar 2023 3:15:33pm
    Author:  romal

  ==============================================================================
*/

#include "Utilities.h"
#include <JuceHeader.h>
string midiIntToString(int midiValue) {

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
    }
    string midiString = noteName.append(noteInt);
    return midiString;
}


int midiStringToInt(string midiValue) {
    //string can be some combination of (Note char)(optional sharp)(optional negative symbol)(note int), giving us 4 possible ways to parse a midi note

    string noteName;
    string noteValue;
    int noteMultiplier;
    int noteAddition;

    if (midiValue[0] != NULL) {
        noteName = midiValue[0];
    }
    else
        return -1;
    if (midiValue[1] != NULL) {
        if (midiValue[1] == '#') {
            noteName.append("#");
            if (&midiValue[2] != NULL) {
                noteValue.append(&midiValue[2]);
            }
            else 
                return -1;
        }
        else {
            noteValue.append(&midiValue[1]);
        }
    }
    else
        return -1;


    if (noteName == "C") {
        noteAddition = 0;
    }
    else if (noteName == "C#") {
        noteAddition = 1;
    }
    else if (noteName == "D") {
        noteAddition = 2;
    }
    else if (noteName == "D#") {
        noteAddition = 3;
    }
    else if (noteName == "E") {
        noteAddition = 4;
    }
    else if (noteName == "F") {
        noteAddition = 5;
    }
    else if (noteName == "F#") {
        noteAddition = 6;
    }
    else if (noteName == "G") {
        noteAddition = 7;
    }
    else if (noteName == "G#") {
        noteAddition = 8;
    }
    else if (noteName == "A") {
        noteAddition = 9;
    }
    else if (noteName == "A#") {
        noteAddition = 10;
    }
    else if (noteName == "B") {
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

    return 12 * noteMultiplier + noteAddition;
}