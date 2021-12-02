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

 /*,
    bpmSlider(*audioProcessor.apvts.getParameter("BPM"), ""),
    subdivisionSlider(*audioProcessor.apvts.getParameter("SUBDIVISION"), ""),
    numeratorSlider(*audioProcessor.apvts.getParameter("NUMERATOR"), ""),
    bpmAttachment(audioProcessor.apvts, "BPM",bpmSlider),
    subdivisionAttachment(audioProcessor.apvts, "SUBDIVISION", subdivisionSlider),
   numeratorAttachment(audioProcessor.apvts, "NUMERATOR", numeratorSlider)*/
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    playButton.setRadioGroupId(1);
    playButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    playButton.onClick = [this]() { play(); }; 
    addAndMakeVisible(playButton);


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
    flexBox.performLayout(bounds);

}

void MetroGnomeAudioProcessorEditor::play()
{

    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
        audioProcessor.metronome.reset();
    }
    else {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    }
}


std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getComps() {
    return{
         &playButton, &bpmSlider, &subdivisionSlider, &numeratorSlider,
    };

}