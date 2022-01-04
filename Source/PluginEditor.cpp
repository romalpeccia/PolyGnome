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


    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);

    playButton.onClick = [this]() { play(); }; 
    defaultModeButton.onClick = [this]() { 
        audioProcessor.apvts.getRawParameterValue("MODE")->store(0);

    };    
    polyRModeButton.onClick = [this]() { 
        audioProcessor.apvts.getRawParameterValue("MODE")->store(1);
    };    
    polyMModeButton.onClick = [this]() { 
        audioProcessor.apvts.getRawParameterValue("MODE")->store(2);
    };

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }



    startTimerHz(60);
    setSize (1000, 700);
}

MetroGnomeAudioProcessorEditor::~MetroGnomeAudioProcessorEditor()
{
}

//==============================================================================
void MetroGnomeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.drawImageAt(logo, 0, 0);

    auto mode = audioProcessor.apvts.getRawParameterValue("MODE")->load();

    if (mode == 0) {

        paintDefaultMode(g);
    }
    else if (mode == 1) {
       // paintDefaultMode(g);
        ;
    }
    else if (mode == 2) {
      //  paintDefaultMode(g);
        ;
    }

}

void MetroGnomeAudioProcessorEditor::paintDefaultMode(juce::Graphics& g) {


    auto bounds = getLocalBounds();
    //response area consists of middle third of top third of area (9 quadrants)
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    responseArea.removeFromLeft(responseArea.getWidth() * 0.33);
    responseArea.removeFromRight(responseArea.getWidth() * 0.5);


    g.setColour(juce::Colours::white);


    //uncomment for debugging purposes
    g.drawRect(responseArea);


    int circleradius = 30;
    auto Y = responseArea.getCentreY();
    auto X = responseArea.getX();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();
    for (int i = 1; i <= audioProcessor.metronome.getNumerator(); i++) {
        //loop to draw metronome circles
        auto circleX = X + i * (circleradius + 5);

        if (audioProcessor.metronome.getoneflag() == i && ON)
        {

            if (audioProcessor.metronome.getbeatflag() != 1)
            {
                g.setColour(juce::Colours::blue);
            }
            else
            {
                g.setColour(juce::Colours::green);
            }

            g.fillEllipse(circleX, Y, circleradius, circleradius);
            g.setColour(juce::Colours::orange);

            g.drawText(juce::String(audioProcessor.metronome.getbeatflag()), circleX, Y, circleradius, circleradius, juce::Justification::centred);
        }
        else
        {
            g.setColour(juce::Colours::white);
            g.fillEllipse(circleX, Y, circleradius, circleradius);
        }

    }
    Y += 100;
    circleradius = 10;
    X = responseArea.getX();
    int subdivisions = audioProcessor.metronome.getSubdivisions();
    int linewidth = 2;


    if (subdivisions != 1)
    {   //loop to draw subdivisions
        for (int i = 1; i <= subdivisions; i++)
        {

            X += circleradius * 3;
            g.setColour(juce::Colours::white);
            if (subdivisions != i)
            {
                //draw horizontal rectangle of note
                g.fillRect(X + circleradius - 3, Y - circleradius - 3, circleradius * 3, linewidth);
            }
            //draw vertical rectangle of note
            g.fillRect(X + circleradius - 3, Y - circleradius - 3, linewidth, circleradius * 2);


            if (audioProcessor.metronome.getbeatflag() == i && ON)
            {
                //fill in the note that is currently being played
                g.setColour(juce::Colours::orange);

            }
            g.fillEllipse(X, Y, circleradius, circleradius);
            // g.fillEllipse(X + (5 * circleradius), Y, circleradius, circleradius);
        }
    }


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
    flexBox.items.add(juce::FlexItem(75, 50, defaultModeButton));
    flexBox.items.add(juce::FlexItem(100, 50, polyRModeButton));
    flexBox.items.add(juce::FlexItem(125, 50, polyMModeButton));
    flexBox.items.add(juce::FlexItem(150, 50, placeholderButton));
    flexBox.performLayout(playBounds);

    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    bounds.removeFromTop(5);

    //every time we remove from bounds, the area of bounds changes 
    //first we remove 1/3 * 1 unit area, then we remove 0.5 * 2/3rd unit area
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    bpmSlider.setBounds(lowCutArea);
    subdivisionSlider.setBounds(highCutArea);
    numeratorSlider.setBounds(bounds);
}

void MetroGnomeAudioProcessorEditor::play()
{
    audioProcessor.metronome.resetall();
    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
    }
    else {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    }
}


std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getComps() {
    return{
         &playButton,
         &defaultModeButton,
         &polyRModeButton,
         &polyMModeButton,
         &placeholderButton,
         
         &bpmSlider, &subdivisionSlider, &numeratorSlider
    };

}

/*
std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getdefaultComps() {
    return{
        &bpmSlider, &subdivisionSlider, &numeratorSlider
    }
}

std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getpolyRComps() {

}
std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getpolyMComps() {

}

*/