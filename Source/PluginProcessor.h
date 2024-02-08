/*(
  ==============================================================================

    This file handles the backend of the PolyGnome, such as the creation of
    parameters, processing of parameters and DAW variables, and an instantion
    of the PolyRhythmMachine class to process timing of MIDI notes.

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
#include "PolyRhythmMachine.h"
#include "Utilities.h"


//==============================================================================
/**
*/
class PolyGnomeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PolyGnomeAudioProcessor();
    ~PolyGnomeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;




    //==============================================================================
    //non default processor code
 
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    

    PolyRhythmMachine polyRhythmMachine{ &apvts };


    // set to false when the 
    bool resetBarAfterPause = false;

    //midiNeedsClearing = true when the machine is turned on, which then triggers all midi notes to be cleared once the machine is turned off, then midiNeedsClearing = false
    bool midiNeedsClearing = false;

    //this keyboardState holds all the notes being played by the machine, which is then copied into the keyboardState of the keyboardComponent in the pluginEditor
    juce::MidiKeyboardState keyboardState;

    //holds the last note pressed by the user on their MIDI instrument. set to -1 when a MIDI textEditor isn't selected
    int storedMidiFromKeyboard = -1;

private:


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyGnomeAudioProcessor)
};
