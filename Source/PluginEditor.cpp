/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace std;
//==============================================================================
PolyGnomeAudioProcessorEditor::PolyGnomeAudioProcessorEditor(PolyGnomeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);

    //initialize the menu buttons
    playButton.onClick = [this]() { 

        togglePlayState();
        toggleAudioProcessorChildrenStates();
    };
    playButton.setButtonText("Play");
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
    playButton.setHelpText(PLAY_BUTTON_REMINDER);

    loadPresetButton.onClick = [this]() {
        loadPreset();
    };
    loadPresetButton.setButtonText("Load Preset");
    loadPresetButton.setHelpText(LOAD_PRESET_BUTTON_REMINDER);

    savePresetButton.onClick = [this]() {
        savePreset();
    };
    savePresetButton.setButtonText("Save Preset");
    savePresetButton.setHelpText(SAVE_PRESET_BUTTON_REMINDER);

    //initialize the bar options
    colorSlider(barSlider, MAIN_COLOUR, MAIN_COLOUR, SECONDARY_COLOUR, MAIN_COLOUR, true);
    barSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NUM_BARS", barSlider);
    barSlider.setHelpText(RACK_SLIDER_REMINDER);
    barSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, 25);

    autoLoopButton.onClick = [this]() {
        if (audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->load() == true) {
            audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->store(false);
        }
        else {
            audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->store(true);
        }
    };
    autoLoopButton.setHelpText(AUTO_LOOP_REMINDER);
    autoLoopButton.setButtonText("Auto-Loop");
    for (int i = 0; i < MAX_BARS; i++)
    { //initialize the bar buttons
        barSelectButtons[i].onClick = [this, i]() {
            audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->store(i);
        };
        barSelectButtons[i].setHelpText(BAR_SELECT_BUTTON_REMINDER);
        barSelectButtons[i].setButtonText(to_string(i+1));
    }

    for (int k = 0; k < MAX_BARS; k++)
    { //initialize the copy bar to other bar buttons
        barCopyButtons[k].onClick = [this, k]() {
            int targetBar = k;
            int currentBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();
            for (int i = 0; i < MAX_TRACKS; i++)
            {

                int subdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(currentBar, i))->load();
                audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(targetBar, i))->store(subdivisions);
                bars[targetBar].tracks[i].subdivisionSlider.setValue(subdivisions);

                int midiValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(currentBar, i))->load();
                audioProcessor.apvts.getRawParameterValue(getMidiValueString(targetBar, i))->store(midiValue);
                bars[targetBar].tracks[i].midiSlider.setValue(midiValue);
                bars[targetBar].tracks[i].midiTextEditor.setText(midiIntToString(midiValue) + " / " + to_string(midiValue));

                int velocity = audioProcessor.apvts.getRawParameterValue(getVelocityString(currentBar, i))->load();
                audioProcessor.apvts.getRawParameterValue(getVelocityString(targetBar, i))->store(velocity);
                bars[targetBar].tracks[i].velocitySlider.setValue(velocity);

                float sustain = audioProcessor.apvts.getRawParameterValue(getSustainString(currentBar, i))->load();
                audioProcessor.apvts.getRawParameterValue(getSustainString(targetBar, i))->store(sustain);
                bars[targetBar].tracks[i].sustainSlider.setValue(sustain);

                bool trackEnabled = audioProcessor.apvts.getRawParameterValue(getTrackEnableString(currentBar, i))->load();
                audioProcessor.apvts.getRawParameterValue(getTrackEnableString(targetBar, i))->store(trackEnabled);
                bars[targetBar].tracks[i].muteButton.setToggleState(trackEnabled, juce::NotificationType::sendNotification);

                /* //TODO: (maybe) copy these as well depending on user feedback
                for (int j = 0; j < MAX_SUBDIVISIONS; j++)
                {
                    bool beatEnabled = audioProcessor.apvts.getRawParameterValue(getBeatToggleString(currentBar, i, j))->load();
                    audioProcessor.apvts.getRawParameterValue(getBeatToggleString(targetBar, i, j))->store(beatEnabled);
                }
                */
            }


        };
        barCopyButtons[k].setHelpText(BAR_COPY_BUTTON_REMINDER);
        barCopyButtons[k].setButtonText(to_string(k + 1));
        barSelectButtons[k].setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
    }

    //initialize the polytrack Machine buttons and sliders
    for (int k = 0; k < MAX_BARS; k++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            for (int j = 0; j < MAX_SUBDIVISIONS; j++)
            { //initialize the track buttons
                juce::String name = getBeatToggleString(k, i, j);
                bars[k].tracks[i].beatButtons[j].onClick = [this, name, k, i , j]() {
                    if (audioProcessor.apvts.getRawParameterValue(name)->load() == true) {
                        bars[k].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                    }
                    else {
                        bars[k].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                    }
                };

                bars[k].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                bars[k].tracks[i].beatButtons[j].setClickingTogglesState(true); 
                bars[k].tracks[i].beatButtonAttachments[j] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, name, bars[k].tracks[i].beatButtons[j]);
                bars[k].tracks[i].beatButtons[j].setHelpText(BEAT_BUTTON_REMINDER);
            }

            //initialize the mute buttons
            bars[k].tracks[i].muteButton.setClickingTogglesState(true);
            bars[k].tracks[i].muteButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, getTrackEnableString(k, i), bars[k].tracks[i].muteButton);
            bars[k].tracks[i].muteButton.setHelpText(MUTE_BUTTON_REMINDER);

            //initialize the subdivision sliders
            bars[k].tracks[i].subdivisionSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            colorSlider(bars[k].tracks[i].subdivisionSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
            bars[k].tracks[i].subdivisionSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getSubdivisionsString(k, i), bars[k].tracks[i].subdivisionSlider);
            bars[k].tracks[i].subdivisionSlider.setHelpText(SUBDIVISION_SLIDER_REMINDER);

            //initialize the velocity sliders
            bars[k].tracks[i].velocitySlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            colorSlider(bars[k].tracks[i].velocitySlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
            bars[k].tracks[i].velocitySliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getVelocityString(k, i), bars[k].tracks[i].velocitySlider);
            bars[k].tracks[i].velocitySlider.setHelpText(VELOCITY_SLIDER_REMINDER);

            //initialize the sustain sliders
            colorSlider(bars[k].tracks[i].sustainSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
            bars[k].tracks[i].sustainSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getSustainString(k, i), bars[k].tracks[i].sustainSlider);
            bars[k].tracks[i].sustainSlider.setHelpText(SUSTAIN_SLIDER_REMINDER);


            //initialize the MIDI control slider    
            bars[k].tracks[i].midiSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
            bars[k].tracks[i].midiSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getMidiValueString(k, i), bars[k].tracks[i].midiSlider);
            bars[k].tracks[i].midiSlider.setHelpText(MIDI_SLIDER_REMINDER);

            //initialize the text entry logic and UI for MIDI Values
            int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->load();
            bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
            bars[k].tracks[i].midiTextEditor.setReadOnly(false);
            colorTextEditor(bars[k].tracks[i].midiTextEditor, ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, true);
            bars[k].tracks[i].midiTextEditor.setHelpText(MIDI_TEXTEDITOR_REMINDER);


            bars[k].tracks[i].midiTextEditor.onReturnKey = [this, k, i]() {
                juce::String input = bars[k].tracks[i].midiTextEditor.getText();
                string inputString = input.toStdString();
                int inputInt;
                string convertedString;
                int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->load();
                if (sscanf(inputString.c_str(), "%d", &inputInt) == 1)
                {   //if the user inputted an int
                    convertedString = midiIntToString(inputInt);
                    if (convertedString != "") {
                        bars[k].tracks[i].midiTextEditor.setText(convertedString + " / " + to_string(inputInt));
                        audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->store(inputInt);
                        //add control of MIDI slider to textbox
                        bars[k].tracks[i].midiSlider.setValue(inputInt);
                    }
                    else {
                        bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                    }
                }
                else
                { //user inputted a string
                    int convertedInt = midiStringToInt(inputString);
                    if (convertedInt != -1) {
                        bars[k].tracks[i].midiTextEditor.setText(inputString + " / " + to_string(convertedInt));
                        audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->store(convertedInt);
                        //add control of MIDI slider to textbox
                        bars[k].tracks[i].midiSlider.setValue(convertedInt);
                    }
                    else {
                        bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                    }
                }
            };

            //add control of textbox to MIDI slider
            bars[k].tracks[i].midiSlider.onValueChange = [this, k , i]() {
                int sliderInt = bars[k].tracks[i].midiSlider.getValue();
                bars[k].tracks[i].midiTextEditor.setText(midiIntToString(sliderInt) + " / " + to_string(sliderInt));
            };


        }
    }



    colorTextEditor(reminderTextEditor, REMINDER_COLOUR, juce::Colours::white, juce::Colours::white, BACKGROUND_COLOUR, true);
    reminderTextEditor.setMultiLine(true);


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

PolyGnomeAudioProcessorEditor::~PolyGnomeAudioProcessorEditor()
{
}

void PolyGnomeAudioProcessorEditor::resized()
{
    //TODO: change how all of this is laid out

    juce::Rectangle<int> menuBounds(100, 100);
    menuBounds.removeFromTop(50);

    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.items.add(juce::FlexItem(100, 50, playButton));
    flexBox.items.add(juce::FlexItem(100, 25, loadPresetButton));
    flexBox.items.add(juce::FlexItem(100, 25, savePresetButton));
    flexBox.items.add(juce::FlexItem(100, 200, reminderTextEditor));
    flexBox.items.add(juce::FlexItem(100, 25, autoLoopButton));
    flexBox.items.add(juce::FlexItem(100, 50, barSlider));
    for (int i = 0; i < MAX_BARS; i++) {
        flexBox.items.add(juce::FlexItem(25, 25, barSelectButtons[i]));
    }
    flexBox.items.add(juce::FlexItem(100, 175));
    for (int i = 0; i < MAX_BARS; i++) {
        flexBox.items.add(juce::FlexItem(25, 25, barCopyButtons[i]));
    }
    flexBox.performLayout(menuBounds);
  
}

//==============================================================================


void PolyGnomeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BACKGROUND_COLOUR);
    g.drawImageAt(logo, 0, 0);

    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true) {
        playButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
    }
    else {
        playButton.setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
    }
    if (audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->load() == true) {
        autoLoopButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
    }
    else {
        autoLoopButton.setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
    }


    reminderTextEditor.setText(getCurrentMouseOverText());

    int numBars = audioProcessor.apvts.getRawParameterValue("NUM_BARS")->load();
    int activeBar = audioProcessor.apvts.getRawParameterValue("ACTIVE_BAR")->load();
    int selectedBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();
    for (int j = 0; j < MAX_BARS; j++) {
        if (j >= numBars) {
            barSelectButtons[j].setVisible(false);
            barCopyButtons[j].setVisible(false);
        }
        else {
            if (j == selectedBar) {
                barCopyButtons[j].setVisible(false);
            }
            else {
                barCopyButtons[j].setVisible(true);
            }
            barSelectButtons[j].setVisible(true);
            if (j == activeBar && j == selectedBar) {
                barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, ENABLED_COLOUR);
            }
            else if (j == selectedBar) {
                barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
            }
            else if (j == activeBar) {
                barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
            }
            else {
                barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
            }
        }
    }
    
    paintPolyRhythmMachine(g);
}

juce::Rectangle<int> PolyGnomeAudioProcessorEditor::getVisualArea()
{
    auto bounds = getLocalBounds();
    //visual area consists of middle third of top third of area 
    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    visualArea.removeFromLeft(visualArea.getWidth() * 0.33);
    visualArea.removeFromRight(visualArea.getWidth() * 0.5);
    return visualArea;
}

void PolyGnomeAudioProcessorEditor::paintPolyRhythmMachine(juce::Graphics& g) {

    //TODO look into only calling redraws of specific elements if needed
    auto visualArea = getVisualArea();
    g.setColour(juce::Colours::white);
    g.drawRect(visualArea);

    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y

    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    int spacing = height / MAX_TRACKS;
    Y = Y + spacing;

    //hide any hidden buttons
    for (int j = 0; j < MAX_BARS; j++){
        for (int i = 0; i < MAX_TRACKS; i++) {
            for (auto* comp : getTrackComps(j, i)) {
                comp->setVisible(false);
            }
        }
    }

    int numBars = audioProcessor.apvts.getRawParameterValue("NUM_BARS")->load();
    int activeBar = audioProcessor.apvts.getRawParameterValue("ACTIVE_BAR")->load();
    int selectedBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();
    bool isBarLoopEnabled = audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->load();
    if (isBarLoopEnabled) {
        selectedBar = activeBar;
    }
    //main loop to draw the tracks and all of its components


    for (int i = 0; i < MAX_TRACKS; i++) {

        //draw the components to the left of the tracks
        juce::Rectangle<int> muteButtonBounds(X - 50, Y + spacing * (i - 1) + 13, 50, 50);
            bars[selectedBar].tracks[i].muteButton.setBounds(muteButtonBounds);
            bars[selectedBar].tracks[i].muteButton.setVisible(true);

            juce::Rectangle<int> velocitySliderBounds(X - 125, Y + spacing * (i - 1), 75, 75);
            bars[selectedBar].tracks[i].velocitySlider.setBounds(velocitySliderBounds);
            bars[selectedBar].tracks[i].velocitySlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, 35, spacing / 2);
            bars[selectedBar].tracks[i].velocitySlider.setVisible(true);

        juce::Rectangle<int> sustainSliderBounds(X - 200, Y + spacing * (i - 1) + 13, 50, 50);
        bars[selectedBar].tracks[i].sustainSlider.setBounds(sustainSliderBounds);
        bars[selectedBar].tracks[i].sustainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 35, spacing / 2);
        bars[selectedBar].tracks[i].sustainSlider.setVisible(true);


        //draw the line segments representing each track
        juce::Path trackLine;
        trackLine.addLineSegment(juce::Line<float>(X, Y + spacing * i, X + width, Y + spacing * i), 1.f);
        g.setColour(SECONDARY_COLOUR);
        g.strokePath(trackLine, juce::PathStrokeType(2.0f));


        bool isTrackEnabled = audioProcessor.apvts.getRawParameterValue(getTrackEnableString(selectedBar, i))->load();
        bool isBarEnabled = (selectedBar == activeBar);
        bool isBeatenabled = (isTrackEnabled && isBarEnabled);
        int subdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(selectedBar, i))->load();
        //draw the buttons for each note of the track
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            bars[selectedBar].tracks[i].beatButtons[j].setBounds(pointBounds);
            bars[selectedBar].tracks[i].beatButtons[j].setVisible(true);


            //TODO : clean this?
            if (audioProcessor.apvts.getRawParameterValue(getBeatToggleString(selectedBar, i, j))->load() == true) {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1) {
                    if (isBeatenabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR.brighter(0.9));
                    }

                }
                else {
                    if (isBeatenabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR.brighter(0.9));
                    }
                }
            }
            else {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1 && audioProcessor.polyRhythmMachine.tracks[i].subdivisions != 1) {
                    if (isBeatenabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR.brighter(0.9));
                    }

                }
                else {
                    if (isBeatenabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR.brighter(0.9));
                    }

                }
            }
        }
        //draw the components to the right of the tracks
        juce::Rectangle<int> subdivisionSliderBounds(X + width + 10, Y + spacing * (i - 1), 75, 75);
        bars[selectedBar].tracks[i].subdivisionSlider.setBounds(subdivisionSliderBounds);
        bars[selectedBar].tracks[i].subdivisionSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, spacing / 2);
        bars[selectedBar].tracks[i].subdivisionSlider.setVisible(true);

        juce::Rectangle<int> midiTextEditorBounds(X + width + 10 + 85, Y + spacing * (i - 1) + 25, 75, 25);
        bars[selectedBar].tracks[i].midiTextEditor.setBounds(midiTextEditorBounds);
        bars[selectedBar].tracks[i].midiTextEditor.setVisible(true);

        juce::Rectangle<int> midiSliderBounds(X + width + 10 + 170, Y + spacing * (i - 1) + 13, 50, 50);
        bars[selectedBar].tracks[i].midiSlider.setBounds(midiSliderBounds);
        bars[selectedBar].tracks[i].midiSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        bars[selectedBar].tracks[i].midiSlider.setVisible(true);

        //adjust the colors of any components belonging to tracks
        colorSlider(bars[selectedBar].tracks[i].subdivisionSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(bars[selectedBar].tracks[i].velocitySlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(bars[selectedBar].tracks[i].sustainSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorTextEditor(bars[selectedBar].tracks[i].midiTextEditor, ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, isTrackEnabled);

        //hide any hidden track components TODO: possibly unecessary after changes
        for (int k = 0; k < MAX_SUBDIVISIONS; k++) {
            if (k >= subdivisions) {
                bars[selectedBar].tracks[i].beatButtons[k].setVisible(false);
            }
        }
    }
}




void PolyGnomeAudioProcessorEditor::toggleAudioProcessorChildrenStates()
{
    audioProcessor.polyRhythmMachine.resetAll();
}
void PolyGnomeAudioProcessorEditor::togglePlayState() {

    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true)
    {
        togglePlayStateOff();
    }
    else {
        togglePlayStateOn();
    }
}
void PolyGnomeAudioProcessorEditor::togglePlayStateOff() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(false);
}
void PolyGnomeAudioProcessorEditor::togglePlayStateOn() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
}


std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getTrackComps(int barIndex, int trackIndex) {
    //returns components for track[i]
    std::vector<juce::Component*> comps;

    comps.push_back(&bars[barIndex].tracks[trackIndex].subdivisionSlider);
    comps.push_back(&bars[barIndex].tracks[trackIndex].velocitySlider);
    comps.push_back(&bars[barIndex].tracks[trackIndex].midiSlider);
    comps.push_back(&bars[barIndex].tracks[trackIndex].midiTextEditor);
    comps.push_back(&bars[barIndex].tracks[trackIndex].muteButton);
    comps.push_back(&bars[barIndex].tracks[trackIndex].sustainSlider);
    for (int j = 0; j < MAX_SUBDIVISIONS; j++) {
        comps.push_back(&bars[barIndex].tracks[trackIndex].beatButtons[j]);
    }
    return{ comps };
}

std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getVisibleComps() {
    //returns components that do not need to be hidden

    std::vector<juce::Component*> comps;
    comps.push_back(&playButton);
    comps.push_back(&loadPresetButton);
    comps.push_back(&savePresetButton);
    comps.push_back(&reminderTextEditor);
    comps.push_back(&barSlider);
    comps.push_back(&autoLoopButton);
    for (int i = 0; i < MAX_TRACKS; i++) {

    }
    return{ comps };
}

std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getHiddenComps() {
    //returns components that sometimes have their state changed to hidden

    std::vector<juce::Component*> comps;
    for (int k = 0; k < MAX_BARS; k++){
        for (int i = 0; i < MAX_TRACKS; i++) {

            comps.push_back(&bars[k].tracks[i].subdivisionSlider);
            comps.push_back(&bars[k].tracks[i].velocitySlider);
            comps.push_back(&bars[k].tracks[i].midiSlider);
            comps.push_back(&bars[k].tracks[i].midiTextEditor);
            comps.push_back(&bars[k].tracks[i].muteButton);
            comps.push_back(&bars[k].tracks[i].sustainSlider);




            for (int j = 0; j < MAX_SUBDIVISIONS; j++) {
                comps.push_back(&bars[k].tracks[i].beatButtons[j]);
            }
        }
        comps.push_back(&barSelectButtons[k]);
        comps.push_back(&barCopyButtons[k]);
    }
    return{ comps };
}

std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getAllComps() {
    std::vector<juce::Component*> comps = getHiddenComps();
    std::vector<juce::Component*> comps2 = getVisibleComps();
    comps.insert(comps.end(), comps2.begin(), comps2.end());
    return { comps };
}

void PolyGnomeAudioProcessorEditor::colorSlider(juce::Slider& slider, juce::Colour thumbColour, juce::Colour textBoxTextColour, juce::Colour textBoxBackgroundColour, juce::Colour textBoxOutlineColour, bool trackEnabled) {
    //any further customization to slider colors should be added here
    if (trackEnabled == false) {
        thumbColour = thumbColour.brighter(0.9);
        textBoxTextColour = textBoxTextColour.brighter(0.9);
        textBoxBackgroundColour = textBoxBackgroundColour.brighter(0.9);
        textBoxOutlineColour = textBoxOutlineColour.brighter(0.9);
    }
    slider.setColour(juce::Slider::ColourIds::thumbColourId, thumbColour);
    slider.setColour(juce::Slider::ColourIds::textBoxTextColourId, textBoxTextColour);
    slider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, textBoxBackgroundColour);
    slider.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, textBoxOutlineColour);
}

void PolyGnomeAudioProcessorEditor::colorTextEditor(juce::TextEditor& textEditor, juce::Colour textColour, juce::Colour focusedOutlineColour, juce::Colour outlineColour, juce::Colour backgroundColour, bool trackEnabled) {
    if (trackEnabled == false) {
        textColour = textColour.brighter(0.9);
        focusedOutlineColour = focusedOutlineColour.brighter(0.9);
        outlineColour = outlineColour.brighter(0.9);
        backgroundColour = backgroundColour.brighter(0.9);
    }

    textEditor.setColour(juce::TextEditor::ColourIds::textColourId, textColour);
    textEditor.setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, focusedOutlineColour);
    textEditor.setColour(juce::TextEditor::ColourIds::outlineColourId, outlineColour);
    textEditor.setColour(juce::TextEditor::ColourIds::backgroundColourId, backgroundColour);
}

juce::String PolyGnomeAudioProcessorEditor::getCurrentMouseOverText() {
    juce::String   reminderText = "";

    if (playButton.isHoveredOver == true) {
        reminderText = playButton.getHelpText();
    }
    else  if (loadPresetButton.isHoveredOver == true) {
        reminderText = loadPresetButton.getHelpText();
    }
    else  if (savePresetButton.isHoveredOver == true) {
        reminderText = savePresetButton.getHelpText();
    }
    else if (barSlider.isHoveredOver == true) {
        reminderText = barSlider.getHelpText();
    }
    else if (autoLoopButton.isHoveredOver == true) {
        reminderText = autoLoopButton.getHelpText();
    }
    else {
        for (int j = 0; j < MAX_BARS; j++) {
            for (int i = 0; i < MAX_TRACKS; i++) {
                if (bars[j].tracks[i].muteButton.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].muteButton.getHelpText();
                    break;
                }
                else if (bars[j].tracks[i].subdivisionSlider.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].subdivisionSlider.getHelpText();
                    break;
                }
                else if (bars[j].tracks[i].midiSlider.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].midiSlider.getHelpText();
                    break;
                }
                else if (bars[j].tracks[i].midiTextEditor.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].midiTextEditor.getHelpText();
                    break;
                }
                else if (bars[j].tracks[i].velocitySlider.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].velocitySlider.getHelpText();
                    break;
                }
                else if (bars[j].tracks[i].sustainSlider.isHoveredOver == true) {
                    reminderText = bars[j].tracks[i].sustainSlider.getHelpText();
                    break;
                }
                else {
                    for (int k = 0; k < MAX_SUBDIVISIONS; k++) {
                        if (bars[j].tracks[i].beatButtons[k].isHoveredOver == true) {
                            reminderText = bars[j].tracks[i].beatButtons[k].getHelpText();
                            break;
                        }
                    }
                }
            }
            if (barSelectButtons[j].isHoveredOver == true) {
                reminderText = barSelectButtons[j].getHelpText();
                break;
            }
            if (barCopyButtons[j].isHoveredOver == true) {
                reminderText = barCopyButtons[j].getHelpText();
                break;
            }
        }
    }
    return reminderText;
}



void PolyGnomeAudioProcessorEditor::savePreset() {
    
    fileChooser = std::make_unique<juce::FileChooser>("Save a .pgnome preset file",
        juce::File::getCurrentWorkingDirectory(),
        "*.pgnome");
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

void PolyGnomeAudioProcessorEditor::loadPreset() {
    
        fileChooser = std::make_unique<juce::FileChooser>("Select a .pgnome preset file",
            juce::File::getCurrentWorkingDirectory(),
            "*.pgnome");

        auto folderChooserFlags = juce::FileBrowserComponent::openMode ;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
            {
                auto gnomeFile = chooser.getResult();
                if (gnomeFile != juce::File{}) {
                    audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*juce::XmlDocument::parse(gnomeFile)));
                }
            });
            
}