/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
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




    //midiNeedsClearing = true when the machine is turned on, which then triggers all midi notes to be cleared once the machine is turned off, then midiNeedsClearing = false
    bool midiNeedsClearing = false;

    //this keyboardState holds all the notes being played by the machine, which is then copied into the keyboardState of the keyboardComponent in the pluginEditor
    juce::MidiKeyboardState keyboardState;

    //holds the last note pressed by the user 
    int storedMidiFromKeyboard = -1;

private:


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyGnomeAudioProcessor)
};
