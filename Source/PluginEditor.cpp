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

    ,
    bpmSlider(*audioProcessor.apvts.getParameter("BPM"), "bpm"),
    subdivisionSlider(*audioProcessor.apvts.getParameter("SUBDIVISION"), " "),
    numeratorSlider(*audioProcessor.apvts.getParameter("NUMERATOR"), " ")
    , 

    bpmAttachment(audioProcessor.apvts, "BPM", bpmSlider),
    subdivisionAttachment(audioProcessor.apvts, "SUBDIVISION", subdivisionSlider),
   numeratorAttachment(audioProcessor.apvts, "NUMERATOR", numeratorSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    //TODO don't hardcode this
    /*juce::File logoFile("C:/Users/Romal/Desktop/JUCE_PROJECTS/MetroGnome/Samples.OSRS_gnome.png");
    static juce::ImageCache imagecache;
        imagecache.getFromFile(logoFile);*/
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);


    playButton.setRadioGroupId(1);
    playButton.setToggleState(false, juce::NotificationType::dontSendNotification);
    playButton.onClick = [this]() { play(); }; 
    
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }


    setSize (1000, 700);
}

MetroGnomeAudioProcessorEditor::~MetroGnomeAudioProcessorEditor()
{
}

//==============================================================================
void MetroGnomeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.drawImageAt(logo, 0, 0);
}

void MetroGnomeAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    juce::Rectangle<int> bounds = getLocalBounds();


    juce::Rectangle<int> playBounds(100, 100);
    playBounds.removeFromTop(50);
    playBounds.removeFromRight(50);
    juce::FlexBox flexBox;
    flexBox.items.add(juce::FlexItem(50   , 50, playButton));
    flexBox.performLayout(playBounds);

    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    /*
    
    visualizer goes here
    */

    bounds.removeFromTop(5);


    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);



    bpmSlider.setBounds(lowCutArea);

    subdivisionSlider.setBounds(highCutArea);
    //every time we remove from bounds, the area of bounds changes 
    //first we remove 1/3 * 1 unit area, then we remove 0.5 * 2/3rd unit area
    numeratorSlider.setBounds(bounds);


    

    
    

}

void MetroGnomeAudioProcessorEditor::play()
{

    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
        audioProcessor.metronome.resetall();
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