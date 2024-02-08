/*(
  ==============================================================================

    This file defines the CustomKeyboardComponent class, which is used to 
    display MIDI notes that are being output by the PolyGnome. 

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

        //TODO: custom setting keyboard colors is not working as expected
        setColour(juce::MidiKeyboardComponent::ColourIds::keySeparatorLineColourId, SECONDARY_COLOUR);
        setColour(juce::MidiKeyboardComponent::ColourIds::keyDownOverlayColourId, MAIN_COLOUR);
        setColour(juce::MidiKeyboardComponent::ColourIds::textLabelColourId, ACCENT_COLOUR);
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

