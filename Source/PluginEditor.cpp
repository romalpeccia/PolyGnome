/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MetroGnomeAudioProcessorEditor::MetroGnomeAudioProcessorEditor (MetroGnomeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    playButton.setRadioGroupId(1);
    playButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    playButton.onClick = [this]() { play(); }; 
    addAndMakeVisible(playButton);
    stopButton.setRadioGroupId(1);
    stopButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    stopButton.onClick = [this]() { stop(); };
    addAndMakeVisible(stopButton);

    setSize (400, 300);
}

MetroGnomeAudioProcessorEditor::~MetroGnomeAudioProcessorEditor()
{
}

//==============================================================================
void MetroGnomeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void MetroGnomeAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();


    juce::FlexBox flexBox;
    flexBox.items.add(juce::FlexItem(50   , 50, playButton));
    flexBox.items.add(juce::FlexItem(50, 50, stopButton));
    flexBox.performLayout(bounds);

}

void MetroGnomeAudioProcessorEditor::play()
{

    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    

}
void MetroGnomeAudioProcessorEditor::stop()
{
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
    audioProcessor.metronome.reset();
}