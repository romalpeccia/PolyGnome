/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             HandlingMidiEventsTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Handles incoming midi events.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        CustomKeyboardComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once
#include "Utilities.h"
//==============================================================================
class CustomKeyboardComponent : public juce::Component,
    private juce::MidiInputCallback,
    private juce::MidiKeyboardStateListener
{
public:
    CustomKeyboardComponent()
        : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
        startTime(juce::Time::getMillisecondCounterHiRes() * 0.001)
    {

        setOpaque(true);
        auto midiInputs = juce::MidiInput::getAvailableDevices();

        // find the first enabled device and use that by default
        for (auto input : midiInputs)
        {
            if (deviceManager.isMidiInputDeviceEnabled(input.identifier))
            {
                setMidiInput(midiInputs.indexOf(input));
                break;
            }
        }
            setMidiInput(0);

        addAndMakeVisible(keyboardComponent);
        keyboardState.addListener(this);

        //setSize(600, 400);
    }

    ~CustomKeyboardComponent() override
    {
        keyboardState.removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        keyboardComponent.setBounds(area.removeFromTop(80).reduced(8));
    }


    juce::MidiKeyboardState keyboardState;            
    juce::MidiKeyboardComponent keyboardComponent;  

private:

    /** Starts listening to a MIDI input device, enabling it if necessary. */
//! [setMidiInput]
    void setMidiInput(int index)
    {
        auto list = juce::MidiInput::getAvailableDevices();

        deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);

        auto newInput = list[index];

        if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
            deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

        deviceManager.addMidiInputDeviceCallback(newInput.identifier, this);

        lastInputIndex = index;
    }
    //! [setMidiInput]

        // These methods handle callbacks from the midi device + on-screen keyboard..
    //! [handleIncomingMidiMessage]
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override
    {
        const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
        keyboardState.processNextMidiEvent(message);

    }
    //! [handleIncomingMidiMessage]

    //! [handleNote]
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override
    {
        if (!isAddingFromMidiInput)
        {
            auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
            m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);

        }
    }

    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float /*velocity*/) override
    {
        if (!isAddingFromMidiInput)
        {
            auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
            m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);

        }
    }
    //! [handleNote]

        // This is used to dispach an incoming message to the message thread
    class IncomingMessageCallback : public juce::CallbackMessage
    {
    public:
        IncomingMessageCallback(CustomKeyboardComponent* o, const juce::MidiMessage& m, const juce::String& s)
            : owner(o), message(m), source(s)
        {}

        void messageCallback() override
        {
            if (owner != nullptr) {
                ;
            }
        }

        Component::SafePointer<CustomKeyboardComponent> owner;
        juce::MidiMessage message;
        juce::String source;
    };




        //==============================================================================
    //! [members]
    juce::AudioDeviceManager deviceManager;           // [1]

    int lastInputIndex = 0;                           // [3]
    bool isAddingFromMidiInput = false;               // [4]

  

    juce::TextEditor midiMessagesBox;
    double startTime;
    //! [members]

        //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKeyboardComponent)
};

