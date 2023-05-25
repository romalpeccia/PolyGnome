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
    subdivisionSlider(*audioProcessor.apvts.getParameter("SUBDIVISION"), "subdivisions"),
    numeratorSlider(*audioProcessor.apvts.getParameter("NUMERATOR"), "numerator"),

    bpmAttachment(audioProcessor.apvts, "BPM", bpmSlider),
    subdivisionAttachment(audioProcessor.apvts, "SUBDIVISION", subdivisionSlider),
    numeratorAttachment(audioProcessor.apvts, "NUMERATOR", numeratorSlider)

{
    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);

    //initialize the menu buttons
    playButton.onClick = [this]() { 

        togglePlayState();
        toggleAudioProcessorChildrenStates();
    };

    metronomeButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(0);
        changeMenuButtonColors(&metronomeButton);
        toggleAudioProcessorChildrenStates();
        togglePlayStateOff();
    };
    polyRhythmButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(1);
        changeMenuButtonColors(&polyRhythmButton);
        toggleAudioProcessorChildrenStates();
        togglePlayStateOff();
    };
    polyMeterButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(2);
        changeMenuButtonColors(&polyMeterButton);
        toggleAudioProcessorChildrenStates();
        togglePlayStateOff();
    };
    polyRhythmMachineButton.onClick = [this]() {
        audioProcessor.apvts.getRawParameterValue("MODE")->store(3);
        changeMenuButtonColors(&polyRhythmMachineButton);
        toggleAudioProcessorChildrenStates();
        togglePlayStateOff();
    };

    loadPresetButton.onClick = [this]() {
        loadPreset();
    };
    savePresetButton.onClick = [this]() {
        savePreset();
    };


    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);
    metronomeButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indigo);
    polyRhythmButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);
    polyMeterButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);
    polyRhythmMachineButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);

    //initialize the polyrhythm Metronome buttons
    for (int i = 0; i < MAX_LENGTH; i++) {
        Rhythm1Buttons[i].onClick = [this, i]() {
            if (audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "_TOGGLE")->load() == true) {
                audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "_TOGGLE")->store(false);
            }
            else {
                audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "_TOGGLE")->store(true);
            }
        };
        Rhythm2Buttons[i].onClick = [this, i]() {
            if (audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "_TOGGLE")->load() == true) {
                audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "_TOGGLE")->store(false);
            }
            else {
                audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "_TOGGLE")->store(true);
            }
        };
    }

    //initialize the polyrhythm Machine buttons and sliders
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++)
        { //initialize the track buttons
            juce::String name = "MACHINE" + to_string(i) + "." + to_string(j) + "_TOGGLE";
            polyRhythmMachineButtons[i][j].onClick = [this, name, i, j]() {
                if (audioProcessor.apvts.getRawParameterValue(name)->load() == true) {
                    //audioProcessor.apvts.getRawParameterValue(name)->store(false);
                    polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
                }
                else {
                    //audioProcessor.apvts.getRawParameterValue(name)->store(true);
                    polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::indigo);
                }
            };
            polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
            polyRhythmMachineButtons[i][j].setClickingTogglesState(true);
            polyRhythmMachineButtonAttachments[i][j] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, name,polyRhythmMachineButtons[i][j]);
        }

        //initialize the mute buttons
        polyRhythmMachineMuteButtons[i].setClickingTogglesState(true);
        polyRhythmMachineMuteButtonAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "MACHINE_TRACK_ENABLE" + to_string(i), polyRhythmMachineMuteButtons[i]);


        //initialize the subdivision sliders
        polyRhythmMachineSubdivisionSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::orange);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::orange);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colours::steelblue);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::orange);

        polyRhythmMachineSubdivisionSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "MACHINE_SUBDIVISIONS" + to_string(i),
            polyRhythmMachineSubdivisionSliders[i]);
         

        //initialize the MIDI control slider    
        polyRhythmMachineMidiSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineMidiSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "MACHINE_MIDI_VALUE" + to_string(i),
            polyRhythmMachineMidiSliders[i]);

        //initialize the text entry logic and UI for MIDI Values
        int currentIntValue = audioProcessor.apvts.getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->load();
        polyRhythmMachineMidiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
        polyRhythmMachineMidiTextEditors[i].setReadOnly(false);
        polyRhythmMachineMidiTextEditors[i].setColour(juce::TextEditor::ColourIds::textColourId, juce::Colours::orange);
        polyRhythmMachineMidiTextEditors[i].setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, juce::Colours::orange);
        polyRhythmMachineMidiTextEditors[i].setColour(juce::TextEditor::ColourIds::outlineColourId, juce::Colours::orange);
        polyRhythmMachineMidiTextEditors[i].setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::indigo);
        polyRhythmMachineMidiTextEditors[i].onReturnKey = [this, i]() {
            juce::String input = polyRhythmMachineMidiTextEditors[i].getText();
            string inputString = input.toStdString();
            int inputInt;
            string convertedString;
            int currentIntValue = audioProcessor.apvts.getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->load();
            if (sscanf(inputString.c_str(), "%d", &inputInt) == 1)
            {   //if the user inputted an int
                convertedString = midiIntToString(inputInt);
                if (convertedString != "") {
                    polyRhythmMachineMidiTextEditors[i].setText(convertedString + " / " + to_string(inputInt));
                    audioProcessor.apvts.getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->store(inputInt);
                    //add control of MIDI slider to textbox
                    polyRhythmMachineMidiSliders[i].setValue(inputInt);
                }
                else {
                    polyRhythmMachineMidiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                }
            } 
            else
            { //user inputted a string
                int convertedInt = midiStringToInt(inputString);
                if (convertedInt != -1) {
                    polyRhythmMachineMidiTextEditors[i].setText(inputString + " / " + to_string(convertedInt));
                    audioProcessor.apvts.getRawParameterValue("MACHINE_MIDI_VALUE" + to_string(i))->store(convertedInt);
                    //add control of MIDI slider to textbox
                    polyRhythmMachineMidiSliders[i].setValue(convertedInt);
                }
                else {
                    polyRhythmMachineMidiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                }
            }   
        };

        //add control of textbox to MIDI slider
        polyRhythmMachineMidiSliders[i].onValueChange = [this, i]() {
            int sliderInt = polyRhythmMachineMidiSliders[i].getValue();
            polyRhythmMachineMidiTextEditors[i].setText(midiIntToString(sliderInt) + " / " + to_string(sliderInt));
        };


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
    flexBox.items.add(juce::FlexItem(175, 50, loadPresetButton));
    flexBox.items.add(juce::FlexItem(200, 50, savePresetButton));
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
    if (audioProcessor.apvts.getRawParameterValue("HOST_CONNECTED")->load()){
        bpmSlider.setEnabled(false);
    }


    if (mode == 3) {
        subdivisionSlider.setVisible(false);
        numeratorSlider.setVisible(false);
        paintPolyRhythmMachineMode(g);
    }
    else {
        //reset the hideable polyrhythm buttons
        for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
            for (int j = 0; j < MAX_LENGTH; j++) {
                polyRhythmMachineButtons[i][j].setVisible(false);

            }
            polyRhythmMachineSubdivisionSliders[i].setVisible(false);
            polyRhythmMachineMidiSliders[i].setVisible(false);
            polyRhythmMachineMidiTextEditors[i].setVisible(false);
            polyRhythmMachineMuteButtons[i].setVisible(false);
            //reset the hidden sliders
            subdivisionSlider.setVisible(true);
            numeratorSlider.setVisible(true);
        }
        loadPresetButton.setVisible(false);
        savePresetButton.setVisible(false);
    }

    if (mode == 1) {
        paintPolyRhythmMetronomeMode(g);
    }
    else {
        //reset the hideable polyrhythm buttons
        for (int i = 0; i < MAX_LENGTH; i++) {
            Rhythm1Buttons[i].setVisible(false);
            Rhythm2Buttons[i].setVisible(false);
        }
    }
    if (mode == 0) {

        paintMetronomeMode(g);
    }
    if (mode == 2) {

        ;
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

    loadPresetButton.setVisible(true);
    savePresetButton.setVisible(true);
    //TODO look into only calling redraws of specific elements if needed
    auto visualArea = getVisualArea();

    //uncomment for debugging purposes    
    //g.setColour(juce::Colours::lightgrey);

    g.drawRect(visualArea);

    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y

    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    int spacing = height / MAX_MIDI_CHANNELS;
    Y = Y + spacing;
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {

        //draw the line segments representing each track
        juce::Path rhythmLine;
        rhythmLine.addLineSegment(juce::Line<float>(X, Y + spacing * i, X + width, Y + spacing * i), 1.f);
        g.setColour(juce::Colours::steelblue);
        g.strokePath(rhythmLine, juce::PathStrokeType(2.0f));
        bool trackEnabled = audioProcessor.apvts.getRawParameterValue("MACHINE_TRACK_ENABLE" + to_string(i))->load();

        //draw the buttons for each note of the track
        int subdivisions = audioProcessor.apvts.getRawParameterValue("MACHINE_SUBDIVISIONS" + to_string(i))->load();
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            polyRhythmMachineButtons[i][j].setBounds(pointBounds);
            polyRhythmMachineButtons[i][j].setVisible(true);


            //TODO : clean this?
            if (audioProcessor.apvts.getRawParameterValue("MACHINE" + to_string(i) + "." + to_string(j) + "_TOGGLE")->load() == true) {
                if (j == audioProcessor.polyRhythmMachine.rhythms[i].counter - 1) {
                    if (trackEnabled == true) {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green);
                    }
                    else {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::green.brighter(0.9));
                    }

                }
                else {
                    if (trackEnabled == true){
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::indigo);
                    }
                    else {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::indigo.brighter(0.9));
                    }
                }
            }
            else {
                if (j == audioProcessor.polyRhythmMachine.rhythms[i].counter - 1 && audioProcessor.polyRhythmMachine.rhythms[i].subdivisions != 1) {
                    if (trackEnabled == true) {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey);
                    }
                    else {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::darkgrey.brighter(0.9));
                    }
                    
                }
                else {
                    if (trackEnabled == true) {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey);
                    }
                    else {
                        polyRhythmMachineButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::grey.brighter(0.9));
                    }
                    
                }
            }
        }
        //hide any hidden buttons
        for (int k = subdivisions; k < MAX_LENGTH; k++) {
            polyRhythmMachineButtons[i][k].setVisible(false);
        }

        //draw the sliders/texteditor
        juce::Rectangle<int> subdivisionSliderBounds(X + width + 10, Y + spacing * (i-1) , 75, 75);
        polyRhythmMachineSubdivisionSliders[i].setBounds(subdivisionSliderBounds);
        polyRhythmMachineSubdivisionSliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, spacing / 2);
        polyRhythmMachineSubdivisionSliders[i].setVisible(true);


        juce::Rectangle<int> midiTextEditorBounds(X + width + 10 + 85, Y + spacing * (i-1) + 25, 75, 25);
        polyRhythmMachineMidiTextEditors[i].setBounds(midiTextEditorBounds);
        polyRhythmMachineMidiTextEditors[i].setVisible(true);

               
        juce::Rectangle<int> midiSliderBounds(X + width + 10 + 160, Y + spacing * (i-1) + 13, 50, 50);
        polyRhythmMachineMidiSliders[i].setBounds(midiSliderBounds);
        polyRhythmMachineMidiSliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        polyRhythmMachineMidiSliders[i].setVisible(true);

        juce::Rectangle<int> muteButtonBounds(X - 50, Y + spacing * (i - 1) + 13, 50, 50);
        polyRhythmMachineMuteButtons[i].setBounds(muteButtonBounds);
        polyRhythmMachineMuteButtons[i].setVisible(true);

    }

}

void MetroGnomeAudioProcessorEditor::paintPolyRhythmMetronomeMode(juce::Graphics& g)
{

    //uncomment for debugging purposes    
    //g.drawRect(visualArea);

    g.setColour(juce::Colours::lightgrey);
    int rhythm1Value = audioProcessor.apvts.getRawParameterValue("NUMERATOR")->load();
    int rhythm2Value = audioProcessor.apvts.getRawParameterValue("SUBDIVISION")->load();
    auto visualArea = getVisualArea();


    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    int circleSkew = 1.2; //TODO adjust this using JUCE_LIVE_CONSTANT
    int radius = ((width > height) ? height : width) / circleSkew; //take the lesser of height and width of the visual area to be our circle diameter
    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y
    if (rhythm1Value != 1)
    {
        drawPolyRhythmCircle(g, radius, width, height, X, Y, rhythm1Value, 1, juce::Colours::lightgrey, juce::Colours::orange, 1);
    }
    if (rhythm2Value != 1)
    {
        drawPolyRhythmCircle(g, radius, width, height, X, Y, rhythm2Value, 1.5, juce::Colours::orange, juce::Colours::lightgrey, 2);
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

void MetroGnomeAudioProcessorEditor::drawPolyRhythmCircle(juce::Graphics& g, int radius, int width, int height, int X, int Y, int rhythmValue, float radiusSkew, juce::Colour circleColour, juce::Colour handColour, int index) {
    // draws the perimiter of a circle with buttons on the line representing beats of the polyrythm, as well as the clock hand indicating which beat is being counted
    // could potentially be adapted to include more than 2 indexes 

    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();
    //draw the circle
    juce::Path rhythmCircle;
    int rhythmRadius = radius / radiusSkew; //TODO maybe change this number, make a param?
    int Xoffset = (width - rhythmRadius) / 2;
    int Yoffset = (height - rhythmRadius) / 2;
    rhythmCircle.addEllipse(X + Xoffset, Y + Yoffset, rhythmRadius, rhythmRadius);
    g.setColour(circleColour);
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
            if ((audioProcessor.apvts.getRawParameterValue("RHYTHM1." + to_string(i) + "_TOGGLE")->load() == true) && audioProcessor.polyRhythmMetronome.getRhythm1Counter() == i) {
                Rhythm1Buttons[i].setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::green);
            }
            else {
                 Rhythm1Buttons[i].setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::grey);
            }
        }
        else if (index == 2) {
            Rhythm2Buttons[i].setBounds(pointBounds);
            Rhythm2Buttons[i].setVisible(true);

            if ((audioProcessor.apvts.getRawParameterValue("RHYTHM2." + to_string(i) + "_TOGGLE")->load() == true) && audioProcessor.polyRhythmMetronome.getRhythm2Counter() == i){
                Rhythm2Buttons[i].setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::green);
            }
            else {
                Rhythm2Buttons[i].setColour(juce::ToggleButton::ColourIds::tickColourId, juce::Colours::grey);
            }
        }

    }
    //draw the clock hand
    g.setColour(handColour);
    float angle = 0;
    juce::Point<int> center;
    if (index == 1) {
        center.setXY(X + rhythmRadius / 2, Y + (height - Xoffset) / 2);
        angle = juce::degreesToRadians(360 * (float(audioProcessor.polyRhythmMetronome.getRhythm1Counter()) / float(rhythmValue)) + 180);
    }
    else if (index == 2) {
        center.setXY(X + Xoffset + rhythmRadius / 2, Y + Yoffset + rhythmRadius / 2);
        angle = juce::degreesToRadians(360 * (float(audioProcessor.polyRhythmMetronome.getRhythm2Counter()) / float(rhythmValue)) + 180);
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
    //g.setColour(juce::Colours::lightgrey);
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
                g.setColour(juce::Colours::steelblue);
            }
            else
            {
                g.setColour(juce::Colours::green );
            }

            g.fillEllipse(circleX, Y, circleradius, circleradius);
            g.setColour(juce::Colours::orange);
            g.drawText(juce::String(audioProcessor.metronome.getSubdivisionCounter()), circleX, Y, circleradius, circleradius, juce::Justification::centred);
        }
        else
        {
            g.setColour(juce::Colours::lightgrey);
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
            g.setColour(juce::Colours::lightgrey);
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



void MetroGnomeAudioProcessorEditor::toggleAudioProcessorChildrenStates()
{
    audioProcessor.metronome.resetAll();
    audioProcessor.polyRhythmMetronome.resetAll();
    audioProcessor.polyRhythmMachine.resetAll();


}
void MetroGnomeAudioProcessorEditor::togglePlayState() {

    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        togglePlayStateOff();
    }
    else {
        togglePlayStateOn();
    }
}
void MetroGnomeAudioProcessorEditor::togglePlayStateOff() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);
}
void MetroGnomeAudioProcessorEditor::togglePlayStateOn() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indigo);
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
    comps.push_back(&loadPresetButton);
    comps.push_back(&savePresetButton);
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
        comps.push_back(&polyRhythmMachineMidiTextEditors[i]);
        comps.push_back(&polyRhythmMachineMuteButtons[i]);
    }

    return{ comps };
}

void MetroGnomeAudioProcessorEditor::changeMenuButtonColors(juce::TextButton *buttonOn) {
    auto buttonColourId = juce::TextButton::ColourIds::buttonColourId;
    metronomeButton.setColour(buttonColourId, juce::Colours::steelblue);
    polyRhythmButton.setColour(buttonColourId, juce::Colours::steelblue);
    polyMeterButton.setColour(buttonColourId, juce::Colours::steelblue);
    polyRhythmMachineButton.setColour(buttonColourId, juce::Colours::steelblue);
    placeholderButton.setColour(buttonColourId, juce::Colours::steelblue);
    buttonOn->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indigo);
}

void MetroGnomeAudioProcessorEditor::savePreset() {
    
    fileChooser = std::make_unique<juce::FileChooser>("Save a .gnome preset file",
        juce::File::getCurrentWorkingDirectory(),
        "*.gnome");
    auto folderChooserFlags = juce::FileBrowserComponent::saveMode;

    fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser )
        {
            std::unique_ptr< juce::XmlElement > apvtsXML = audioProcessor.apvts.copyState().createXml();
            auto gnomeFile = chooser.getResult();
            if (gnomeFile != juce::File{} ) {
                apvtsXML->writeTo(gnomeFile, juce::XmlElement::TextFormat());
                DBG(apvtsXML->toString());
            }
        });
        
}

void MetroGnomeAudioProcessorEditor::loadPreset() {
    
        fileChooser = std::make_unique<juce::FileChooser>("Select a .gnome preset file",
            juce::File::getCurrentWorkingDirectory(),
            "*.gnome");

        auto folderChooserFlags = juce::FileBrowserComponent::openMode ;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
            {
                auto gnomeFile = chooser.getResult();
                if (gnomeFile != juce::File{}) {
                    audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*juce::XmlDocument::parse(gnomeFile)));
                    audioProcessor.apvts.getRawParameterValue("MODE")->store(3);
                }
            });
            
}