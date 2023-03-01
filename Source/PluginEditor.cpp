/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace std;
//==============================================================================
MetroGnomeAudioProcessorEditor::MetroGnomeAudioProcessorEditor(MetroGnomeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)

    ,
    bpmSlider(*audioProcessor.apvts.getParameter("BPM"), "bpm"),
    subdivisionSlider(*audioProcessor.apvts.getParameter("SUBDIVISION"), " "),
    numeratorSlider(*audioProcessor.apvts.getParameter("NUMERATOR"), " "),

    bpmAttachment(audioProcessor.apvts, "BPM", bpmSlider),
    subdivisionAttachment(audioProcessor.apvts, "SUBDIVISION", subdivisionSlider),
    numeratorAttachment(audioProcessor.apvts, "NUMERATOR", numeratorSlider)

{


    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);

    playButton.onClick = [this]() { play(); };
    metronomeButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(0);

    };
    polyRhythmButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(1);

    };
    polyMeterButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(2);

    };
    polyRhythmMachineButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(3);

    };


    for (int i = 0; i < MAX_LENGTH; i++) {
        Rhythm1Buttons[i].onClick = [this, i]() {
            if (audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "TOGGLE")->load() == true) {
                audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "TOGGLE")->store(false);
            }
            else {
                audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "TOGGLE")->store(true);
            }
        };
        Rhythm2Buttons[i].onClick = [this, i]() {
            if (audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "TOGGLE")->load() == true) {
                audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "TOGGLE")->store(false);
            }
            else {
                audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "TOGGLE")->store(true);
            }
        };
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++)
        {
            juce::String name = "MACHINE" + to_string(i) + "." + to_string(j) + "TOGGLE";
            polyRhythmMachineButtons[i][j].onClick = [this, name]() {
                if (audioProcessor.apvts.getRawParameterValue(name)->load() == true) {
                    audioProcessor.apvts.getRawParameterValue(name)->store(false);
                }
                else {
                    audioProcessor.apvts.getRawParameterValue(name)->store(true);
                }
            };
        }
        polyRhythmMachineSubdivisionSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineSubdivisionSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "MACHINESUBDIVISIONS" + to_string(i),
            polyRhythmMachineSubdivisionSliders[i]);

        polyRhythmMachineMidiSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineMidiSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "MACHINEMIDI" + to_string(i),
            polyRhythmMachineMidiSliders[i]);



    }


    for (auto* comp : getVisibleComps())
    {
        addAndMakeVisible(comp);
    }
    for (auto* comp : getHiddenComps()) {
        addChildComponent(comp);
    }


    startTimerHz(144);
    setSize(1000, 700);
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
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.items.add(juce::FlexItem(50, 50, playButton));
    flexBox.items.add(juce::FlexItem(75, 50, metronomeButton));
    flexBox.items.add(juce::FlexItem(100, 50, polyRhythmButton));
    flexBox.items.add(juce::FlexItem(125, 50, polyMeterButton));
    flexBox.items.add(juce::FlexItem(150, 50, polyRhythmMachineButton));
    flexBox.performLayout(playBounds);



    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    //unused variable, this call cuts away space used by the visual area from bounds, may be used later


    bounds.removeFromTop(5);
    //every time we remove from bounds, the area of bounds changes 
    //first we remove 1/3 * 1 unit area, then we remove 0.5 * 2/3rd unit area
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto rightArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    bpmSlider.setBounds(leftArea);
    subdivisionSlider.setBounds(rightArea);
    numeratorSlider.setBounds(bounds);
}

MetroGnomeAudioProcessorEditor::~MetroGnomeAudioProcessorEditor()
{
}

//==============================================================================
void MetroGnomeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.drawImageAt(logo, 0, 0);

    auto mode = audioProcessor.apvts.getRawParameterValue("MODE")->load();

    //reset the hideable polyrhythm buttons
    for (int i = 0; i < MAX_LENGTH; i++) {
        Rhythm1Buttons[i].setVisible(false);
        Rhythm2Buttons[i].setVisible(false);
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++) {
            polyRhythmMachineButtons[i][j].setVisible(false);

        }
        polyRhythmMachineSubdivisionSliders[i].setVisible(false);
        polyRhythmMachineMidiSliders[i].setVisible(false);
    }



    if (mode == 0) {

        paintMetronomeMode(g);
    }
    else if (mode == 1) {
        paintPolyRhythmMetronomeMode(g);

    }
    else if (mode == 2) {

        ;
    }
    else if (mode == 3) {
        paintPolyRhythmMachineMode(g);
    }

}

juce::Rectangle<int> MetroGnomeAudioProcessorEditor::getVisualArea()
{
    auto bounds = getLocalBounds();
    //visual area consists of middle third of top third of area 
    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    visualArea.removeFromLeft(visualArea.getWidth() * 0.33);
    visualArea.removeFromRight(visualArea.getWidth() * 0.5);
    return visualArea;
}



void MetroGnomeAudioProcessorEditor::paintPolyRhythmMachineMode(juce::Graphics& g) {

    auto visualArea = getVisualArea();

    //uncomment for debugging purposes    
    //g.setColour(juce::Colours::white);

    g.drawRect(visualArea);

    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y

    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    int spacing = height / MAX_MIDI_CHANNELS;
    Y = Y + spacing;
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {

        //draw the line segments
        juce::Path rhythmLine;
        rhythmLine.addLineSegment(juce::Line<float>(X, Y + spacing * i, X + width, Y + spacing * i), 1.f);
        g.setColour(juce::Colours::blue);
        g.strokePath(rhythmLine, juce::PathStrokeType(2.0f));

        //draw the buttons
        int subdivisions = audioProcessor.apvts.getRawParameterValue("MACHINESUBDIVISIONS" + to_string(i))->load();
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            polyRhythmMachineButtons[i][j].setBounds(pointBounds);
            polyRhythmMachineButtons[i][j].setVisible(true);
        }
        //hide any hidden buttons
        for (int k = subdivisions; k < MAX_LENGTH; k++) {
            polyRhythmMachineButtons[i][k].setVisible(false);
        }

        //draw the sliders
        juce::Rectangle<int> subdivisionSliderBounds(X + width + 10, Y + spacing * i - 35, 75, 75);
        polyRhythmMachineSubdivisionSliders[i].setBounds(subdivisionSliderBounds);
        polyRhythmMachineSubdivisionSliders[i].setVisible(true);

        juce::Rectangle<int> midiSliderBounds(X + width + 10 + 75, Y + spacing * i - 35, 75, 75);
        polyRhythmMachineMidiSliders[i].setBounds(midiSliderBounds);
        polyRhythmMachineMidiSliders[i].setVisible(true);
    }

}

void MetroGnomeAudioProcessorEditor::paintPolyRhythmMetronomeMode(juce::Graphics& g)
{
    auto visualArea = getVisualArea();
    //uncomment for debugging purposes    
    g.setColour(juce::Colours::white);
    //g.drawRect(visualArea);


    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    int circleSkew = 1.2; //TODO adjust this using JUCE_LIVE_CONSTANT
    int radius = ((width > height) ? height : width) / circleSkew;
    //take the lesser of height and width to be our circle diameter
    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y


    int rhythm1Value = audioProcessor.apvts.getRawParameterValue("NUMERATOR")->load();
    int rhythm2Value = audioProcessor.apvts.getRawParameterValue("SUBDIVISION")->load();

    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    //function params radius, width, height, X, Y
    if (rhythm1Value != 1)
    {
        drawPolyRhythmVisual(g, radius, width, height, X, Y, rhythm1Value, 1, juce::Colours::white, juce::Colours::orange, 1);
    }
    if (rhythm2Value != 1)
    {
        drawPolyRhythmVisual(g, radius, width, height, X, Y, rhythm2Value, 1.5, juce::Colours::orange, juce::Colours::white, 2);
    }

    //hide any components that no longer need to be shown
    for (int i = rhythm1Value; i < MAX_LENGTH; i++) {
        Rhythm1Buttons[i].setVisible(false);
    }
    for (int i = rhythm2Value; i < MAX_LENGTH; i++) {
        Rhythm2Buttons[i].setVisible(false);
    }
    if (rhythm1Value == 1) {
        Rhythm1Buttons[0].setVisible(false);
    }
    if (rhythm2Value == 1) {
        Rhythm2Buttons[0].setVisible(false);
    }

}

void MetroGnomeAudioProcessorEditor::drawPolyRhythmVisual(juce::Graphics& g, int radius, int width, int height, int X, int Y, int rhythmValue, float radiusSkew, juce::Colour color1, juce::Colour color2, int index) {
    // draws the perimiter of a circle with buttons on the line representing beats of the polyrythm, as well as the clock hand indicating which beat is being counted

    //draw the circle
    juce::Path rhythmCircle;
    int rhythmRadius = radius / radiusSkew; //TODO maybe change this number, make a param?
    int Xoffset = (width - rhythmRadius) / 2;
    int Yoffset = (height - rhythmRadius) / 2;
    rhythmCircle.addEllipse(X + Xoffset, Y + Yoffset, rhythmRadius, rhythmRadius);
    g.setColour(color1);
    g.strokePath(rhythmCircle, juce::PathStrokeType(2.0f));


    float rhythm1Length = rhythmCircle.getLength();
    for (int i = 0; i < rhythmValue; i++)
    {
        //iterate through beats and draw toggle button on the edge of the main circle

        float distanceOnPath = (float(i) / rhythmValue) * rhythm1Length;
        auto point = rhythmCircle.getPointAlongPath(distanceOnPath);
        juce::Rectangle<int> pointBounds(point.getX(), point.getY(), 22, 22); //TODO  why 22?
        if (index == 1) {
            Rhythm1Buttons[i].setBounds(pointBounds);
            Rhythm1Buttons[i].setVisible(true);
        }
        else if (index == 2) {
            Rhythm2Buttons[i].setBounds(pointBounds);
            Rhythm2Buttons[i].setVisible(true);
        }

    }
    //draw the clock hand
    g.setColour(color2);
    float angle = juce::degreesToRadians(360 * (float(audioProcessor.polyRhythmMetronome.getRhythm1Counter()) / float(rhythmValue)) + 180);
    juce::Point<int> center;
    if (index == 1) {
        center.setXY(X + rhythmRadius / 2, Y + (height - Xoffset) / 2);
    }
    else if (index == 2) {
        center.setXY(X + Xoffset + rhythmRadius / 2, Y + Yoffset + rhythmRadius / 2);
    }

    juce::Path clockHand;
    juce::Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(center.getY());
    r.setBottom(center.getY() + rhythmRadius / 2);
    clockHand.addRoundedRectangle(r, 2.f);
    clockHand.applyTransform(juce::AffineTransform().rotation(angle, center.getX(), center.getY()));
    g.fillPath(clockHand);

}

void MetroGnomeAudioProcessorEditor::paintMetronomeMode(juce::Graphics& g) {


    auto visualArea = getVisualArea();
    //uncomment for debugging purposes
    //g.setColour(juce::Colours::white);
    //g.drawRect(visualArea);


    int circleradius = 30;
    auto Y = visualArea.getCentreY();
    auto X = visualArea.getX();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    for (int i = 1; i <= audioProcessor.metronome.getNumerator(); i++) {
        //loop to draw metronome circles
        auto circleX = X + i * (circleradius + 5);

        if (audioProcessor.metronome.getBeatCounter() == i && ON)
        {

            if (audioProcessor.metronome.getSubdivisionCounter() != 1)
            {
                g.setColour(juce::Colours::blue);
            }
            else
            {
                g.setColour(juce::Colours::green);
            }

            g.fillEllipse(circleX, Y, circleradius, circleradius);
            g.setColour(juce::Colours::orange);
            g.drawText(juce::String(audioProcessor.metronome.getSubdivisionCounter()), circleX, Y, circleradius, circleradius, juce::Justification::centred);
        }
        else
        {
            g.setColour(juce::Colours::white);
            g.fillEllipse(circleX, Y, circleradius, circleradius);
        }

    }

    Y += 100;
    circleradius = 10;
    X = visualArea.getX();
    int subdivisions = audioProcessor.metronome.getSubdivisions();
    int linewidth = 2;


    if (subdivisions > 0)
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


            if (audioProcessor.metronome.getSubdivisionCounter() == i && ON)
            {
                //fill in the note that is currently being played
                g.setColour(juce::Colours::orange);

            }
            g.fillEllipse(X, Y, circleradius, circleradius);
        }
    }
}



void MetroGnomeAudioProcessorEditor::play()
{
    audioProcessor.metronome.resetall();
    audioProcessor.polyRhythmMetronome.resetall();
    audioProcessor.polyRhythmMachine.resetall();
    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
    }
    else {
        audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    }
}


std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getVisibleComps() {


    std::vector<juce::Component*> comps;
    comps.push_back(&playButton);
    comps.push_back(&metronomeButton);
    comps.push_back(&polyRhythmButton);
    comps.push_back(&polyMeterButton);
    comps.push_back(&polyRhythmMachineButton);
    comps.push_back(&bpmSlider);
    comps.push_back(&subdivisionSlider);
    comps.push_back(&numeratorSlider);


    return{ comps };
}

std::vector<juce::Component*> MetroGnomeAudioProcessorEditor::getHiddenComps() {

    std::vector<juce::Component*> comps;
    for (int i = 0; i < MAX_LENGTH; i++) {
        comps.push_back(&Rhythm1Buttons[i]);
        comps.push_back(&Rhythm2Buttons[i]);
    }

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++) {
            comps.push_back(&polyRhythmMachineButtons[i][j]);
        }
        comps.push_back(&polyRhythmMachineSubdivisionSliders[i]);
        comps.push_back(&polyRhythmMachineMidiSliders[i]);
    }

    return{ comps };
}
