/*(
  ==============================================================================

    This file handles the creation and painting of UI components of the PolyGnome. 

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
#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace std;
//==============================================================================


PolyGnomeAudioProcessorEditor::PolyGnomeAudioProcessorEditor(PolyGnomeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    initializeImages();
    initializeMenuComponents();
    initializeMachineComponents();

    //add components to component tree 
    for (auto* comp : getVisibleComps())
    {
        addAndMakeVisible(comp);
    }
    for (auto* comp : getHiddenComps()) {
        addChildComponent(comp);
    }

    startTimerHz(144);
    setSize(PLUGIN_WIDTH, PLUGIN_HEIGHT);
}

PolyGnomeAudioProcessorEditor::~PolyGnomeAudioProcessorEditor()
{

}


void PolyGnomeAudioProcessorEditor::initializeImages()
{
    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);
    keyboardIcon = juce::ImageCache::getFromMemory(BinaryData::keyboard_png, BinaryData::keyboard_pngSize);
    trackIcon = juce::ImageCache::getFromMemory(BinaryData::track_png, BinaryData::track_pngSize);
    sustainIcon = juce::ImageCache::getFromMemory(BinaryData::sustain_png, BinaryData::sustain_pngSize);
    velocityIcon = juce::ImageCache::getFromMemory(BinaryData::velocity_png, BinaryData::velocity_pngSize);
    enableIcon = juce::ImageCache::getFromMemory(BinaryData::enable_png, BinaryData::enable_pngSize);
}



void PolyGnomeAudioProcessorEditor::initializeMenuComponents() {

    //initialize the playButton (currently defunct, may reintroduce it again for standalone mode)
    menu.playButton.onClick = [this]() {
        togglePlayState();
        toggleAudioProcessorChildrenStates();
        };
    menu.playButton.setButtonText("Play");
    menu.playButton.setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
    menu.playButton.setHelpText(PLAY_BUTTON_REMINDER);

    //initialize the load/save preset buttons
    menu.loadPresetButton.onClick = [this]() {
        loadPreset();
        };
    menu.loadPresetButton.setButtonText("Load Preset");
    menu.loadPresetButton.setHelpText(LOAD_PRESET_BUTTON_REMINDER);
    
    menu.savePresetButton.onClick = [this]() {
        savePreset();
        };
    menu.savePresetButton.setButtonText("Save Preset");
    menu.savePresetButton.setHelpText(SAVE_PRESET_BUTTON_REMINDER);
    
    //initialize  barSlider
    colorSlider(menu.barSlider, MAIN_COLOUR, MAIN_COLOUR, SECONDARY_COLOUR, MAIN_COLOUR, true);
    menu.barSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NUM_BARS", menu.barSlider);
    menu.barSlider.setHelpText(BAR_SLIDER_REMINDER);
    menu.barSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, 25);

    //initialize reminderTextEditor
    colorTextEditor(menu.reminderTextEditor, REMINDER_COLOUR, juce::Colours::white, juce::Colours::white, BACKGROUND_COLOUR, true);
    menu.reminderTextEditor.setMultiLine(true);

    //initialize autoloop button
    menu.autoLoopButton.onClick = [this]() {
        if (audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->load() == true) {
            audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->store(false);
        }
        else {
            audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->store(true);
        }
        };
    menu.autoLoopButton.setHelpText(AUTO_LOOP_REMINDER);
    menu.autoLoopButton.setButtonText("Auto-Loop");

    //initialize the bar buttons
    for (int i = 0; i < MAX_BARS; i++)
    {
        menu.barSelectButtons[i].onClick = [this, i]() {
            audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->store(i);
            audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->store(false);
            };
        menu.barSelectButtons[i].setHelpText(BAR_SELECT_BUTTON_REMINDER);
        menu.barSelectButtons[i].setButtonText(to_string(i + 1));
    }

    //initialize the copy bar to other bar buttons
    for (int k = 0; k < MAX_BARS; k++)
    {
        menu.barCopyButtons[k].onClick = [this, k]() {
            int targetBar = k;
            int currentBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();
            for (int i = 0; i < MAX_TRACKS; i++)
            {
                int subdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(currentBar, i))->load();
                bars[targetBar].tracks[i].subdivisionSlider.setValue(subdivisions);

                int midiValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(currentBar, i))->load();
                bars[targetBar].tracks[i].midiSlider.setValue(midiValue);
                bars[targetBar].tracks[i].midiTextEditor.setText(midiIntToString(midiValue) + " | " + to_string(midiValue));

                int velocity = audioProcessor.apvts.getRawParameterValue(getVelocityString(currentBar, i))->load();
                bars[targetBar].tracks[i].velocitySlider.setValue(velocity);

                float sustain = audioProcessor.apvts.getRawParameterValue(getSustainString(currentBar, i))->load();
                bars[targetBar].tracks[i].sustainSlider.setValue(sustain);

                bool trackEnabled = audioProcessor.apvts.getRawParameterValue(getTrackEnableString(currentBar, i))->load();
                bars[targetBar].tracks[i].muteButton.setToggleState(trackEnabled, juce::NotificationType::sendNotification);

                for (int j = 0; j < subdivisions; j++)
                {
                    bool beatEnabled = audioProcessor.apvts.getRawParameterValue(getBeatToggleString(currentBar, i, j))->load();
                    bars[targetBar].tracks[i].beatButtons[j].setToggleState(beatEnabled, juce::NotificationType::sendNotification);
                }
            }
        };
        menu.barCopyButtons[k].setHelpText(BAR_COPY_BUTTON_REMINDER);
        menu.barCopyButtons[k].setButtonText(to_string(k + 1));
    }
}

void PolyGnomeAudioProcessorEditor::initializeMachineComponents() {
    prevBeatID.setbeatID(1, 1, 1);
    for (int k = 0; k < MAX_BARS; k++) {
        for (int i = 0; i < MAX_TRACKS; i++) {
            for (int j = 0; j < MAX_SUBDIVISIONS; j++)
            { //initialize the track buttons
                juce::Font buttonFont = juce::Font(8);
                bars[k].tracks[i].beatButtons[j].setClickingTogglesState(true);
                bars[k].tracks[i].beatButtonAttachments[j] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, getBeatToggleString(k, i, j) , bars[k].tracks[i].beatButtons[j]);
                bars[k].tracks[i].beatButtons[j].setHelpText(BEAT_BUTTON_REMINDER);

                //initialize their submenus
                bars[k].tracks[i].beatMidiSliderAttachments[j] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getBeatMidiString(k, i, j), bars[k].tracks[i].beatMidiSliders[j]);
                bars[k].tracks[i].beatButtons[j].beatId.setbeatID(k, i, j);
                bars[k].tracks[i].beatButtons[j].apvts = &audioProcessor.apvts;
                bars[k].tracks[i].beatButtons[j].selectedBeatPtr = &selectedBeatID;
                bars[k].tracks[i].beatLabels[j].setText("Bar " + to_string(k + 1) + " Track " + to_string(i + 1) + " Beat " + to_string(j + 1), juce::NotificationType::dontSendNotification);
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
            colorSlider(bars[k].tracks[i].midiSlider, MAIN_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
            bars[k].tracks[i].midiSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getMidiValueString(k, i), bars[k].tracks[i].midiSlider);
            bars[k].tracks[i].midiSlider.setHelpText(MIDI_SLIDER_REMINDER);

            //initialize the text entry logic and UI for MIDI Values
            int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->load();
            bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " | " + to_string(currentIntValue));
            bars[k].tracks[i].midiTextEditor.setReadOnly(false);
            colorTextEditor(bars[k].tracks[i].midiTextEditor, ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, true);
            bars[k].tracks[i].midiTextEditor.setHelpText(MIDI_TEXTEDITOR_REMINDER);

            //add control of textbox value to MIDI slider
            bars[k].tracks[i].midiSlider.onValueChange = [this, k, i]() {
                int sliderInt = bars[k].tracks[i].midiSlider.getValue();
                bars[k].tracks[i].midiTextEditor.setText(midiIntToString(sliderInt) + " | " + to_string(sliderInt));
                for (int z = 0; z < MAX_SUBDIVISIONS; z++) {
                    bars[k].tracks[i].beatMidiSliders[z].setValue(sliderInt);
                }
            };
            //initialize the MIDITextEditor's text entry function
            bars[k].tracks[i].midiTextEditor.onReturnKey = [this, k, i]() {
                bars[k].tracks[i].midiTextEditor.giveAwayKeyboardFocus();
                juce::String input = bars[k].tracks[i].midiTextEditor.getText();
                string inputString = input.toStdString();
                int inputInt;
                string convertedString;
                int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(k, i))->load();
                if (sscanf(inputString.c_str(), "%d", &inputInt) == 1){   
                    //if user inputted an int
                    convertedString = midiIntToString(inputInt);
                    if (convertedString != "") {
                        bars[k].tracks[i].midiTextEditor.setText(convertedString + " | " + to_string(inputInt));
                        //add control of MIDI slider value to textbox
                        bars[k].tracks[i].midiSlider.setValue(inputInt, juce::NotificationType::sendNotification);
                    }
                    else {
                        bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " | " + to_string(currentIntValue));
                    }
                }
                else{ 
                    //if user inputted a string
                    int convertedInt = midiStringToInt(inputString);
                    if (convertedInt != -1) {
                        bars[k].tracks[i].midiTextEditor.setText(inputString + " | " + to_string(convertedInt));
                        //add control of MIDI slider to textbox
                        bars[k].tracks[i].midiSlider.setValue(convertedInt, juce::NotificationType::sendNotification);
                    }
                    else {
                        bars[k].tracks[i].midiTextEditor.setText(midiIntToString(currentIntValue) + " | " + to_string(currentIntValue));
                    }
                }
            };
        }
    }
}



void PolyGnomeAudioProcessorEditor::resized()
{
    
    juce::Rectangle<int> menuBounds(MENU_WIDTH, MENU_WIDTH);
    menuBounds.removeFromTop(50);
    
    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 50, menu.playButton));
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 25, menu.loadPresetButton));
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 25, menu.savePresetButton));
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 200, menu.reminderTextEditor));
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 25, menu.autoLoopButton));
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 50, menu.barSlider));
    for (int i = 0; i < MAX_BARS; i++) {
        flexBox.items.add(juce::FlexItem(25, 25, menu.barSelectButtons[i]));
    }
    flexBox.items.add(juce::FlexItem(MENU_WIDTH, 175));
    for (int i = 0; i < MAX_BARS; i++) {
        flexBox.items.add(juce::FlexItem(25, 25, menu.barCopyButtons[i]));
    }
    flexBox.performLayout(menuBounds);
    
    juce::FlexBox flexBox2;
    juce::Rectangle<int> keyboardBounds(PLUGIN_WIDTH, PLUGIN_HEIGHT);
    flexBox2.items.add(juce::FlexItem(PLUGIN_WIDTH - MENU_WIDTH, 200, keyboard));
    keyboardBounds.removeFromTop(PLUGIN_HEIGHT - 75);
    keyboardBounds.removeFromLeft(MENU_WIDTH);
    flexBox2.performLayout(keyboardBounds);
}

void PolyGnomeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BACKGROUND_COLOUR);

    auto visualArea = getVisualArea();
    g.drawImageAt(logo, 0, 0);
    g.drawImageAt(sustainIcon, visualArea.getX() - 190, 8);
    g.drawImageAt(velocityIcon, visualArea.getX() - 105, 8);
    g.drawImageAt(enableIcon, visualArea.getX() - 50, 8);
    g.drawImageAt(trackIcon, visualArea.getX() + visualArea.getWidth() + 35, 5);
    g.drawImageAt(keyboardIcon, visualArea.getX() + visualArea.getWidth() + 155, 5);


    if (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true) {
        menu.playButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
    }
    else {
        menu.playButton.setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
    }


    if (audioProcessor.apvts.getRawParameterValue("AUTO_LOOP")->load() == true) {
        menu.autoLoopButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
    }
    else {
        menu.autoLoopButton.setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
    }

    menu.reminderTextEditor.setText(getCurrentMouseOverText());


    int numBars = audioProcessor.apvts.getRawParameterValue("NUM_BARS")->load();
    int activeBar = audioProcessor.apvts.getRawParameterValue("ACTIVE_BAR")->load();
    int selectedBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();

    //handle the notes being output by the machine
    if (audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->load() == -1) {
        for (int i = 0; i < MAX_MIDI_VALUE; i++) {

            if (audioProcessor.keyboardState.isNoteOnForChannels(0xffff, i)) {
                keyboard.keyboardState.noteOn(MIDI_CHANNEL, i, 1.f);
            }
            else {
                keyboard.keyboardState.noteOff(MIDI_CHANNEL, i, 1.f);
            }
        }
    }
    
    //handle notes being input by user through the MIDI text editor

    //check if a midi texteditor is selected, and save that information in our apvts
    int isMidiSelected = false;
    for (int i = 0; i < MAX_TRACKS; i++) {
        if (bars[selectedBar].tracks[i].midiTextEditor.isFocussed) {
            audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->store(i);
            isMidiSelected = true;
            break;
        }
    }
    if (isMidiSelected == false) {
        audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->store(-1);
    }
    
    //if the user has entered a MIDI note, update the UI accordingly
    int selectedMidi = audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->load();
    if (audioProcessor.storedMidiFromKeyboard != -1 ) {
        DBG("TEST");
        //if a midiTextEditor is selected, change that note, otherwise, change the currently selected beat's note
        if (selectedMidi != -1) {
            bars[selectedBar].tracks[selectedMidi].midiTextEditor.setText(midiIntToString(audioProcessor.storedMidiFromKeyboard) + " | " + to_string(audioProcessor.storedMidiFromKeyboard));
            bars[selectedBar].tracks[selectedMidi].midiSlider.setValue(audioProcessor.storedMidiFromKeyboard);
            //give focus to the next MIDI texteditor in the queue
            if (selectedMidi < MAX_TRACKS - 1) {
                audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->store(selectedMidi + 1);
                bars[selectedBar].tracks[selectedMidi + 1].midiTextEditor.grabKeyboardFocus();
            }
            else {
                audioProcessor.apvts.getRawParameterValue("SELECTED_MIDI_TRACK")->store(0);
                bars[selectedBar].tracks[0].midiTextEditor.grabKeyboardFocus();
            }
        }
        else {
            int numSubdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(selectedBar, selectedBeatID._trackID))->load();
            bars[selectedBar].tracks[selectedBeatID._trackID].beatMidiSliders[selectedBeatID._beatID].setValue(audioProcessor.storedMidiFromKeyboard);
            if (selectedBeatID._beatID < numSubdivisions - 1) {
                selectedBeatID._beatID += 1;
            }
            else {
                selectedBeatID._beatID = 0;
            }

        }




        audioProcessor.storedMidiFromKeyboard = -1;

    }

    //draw all the buttons related to bar selection/copying
    for (int j = 0; j < MAX_BARS; j++) {
        if (j >= numBars) {
            menu.barSelectButtons[j].setVisible(false);
            menu.barCopyButtons[j].setVisible(false);
        }
        else {
            if (j == selectedBar) {
                menu.barCopyButtons[j].setVisible(false);
            }
            else {
                menu.barCopyButtons[j].setVisible(true);
            }
            menu.barSelectButtons[j].setVisible(true);
            if (j == activeBar && j == selectedBar) {
                menu.barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, ENABLED_COLOUR);
            }
            else if (j == selectedBar) {
                menu.barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
            }
            else if (j == activeBar) {
                menu.barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR);
            }
            else {
                menu.barSelectButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
            }
        }
    }
    
    paintPolyRhythmMachine(g);

    if (selectedBeatID._barID != prevBeatID._barID || selectedBeatID._trackID != prevBeatID._trackID || selectedBeatID._beatID != prevBeatID._beatID) {
        bars[prevBeatID._barID].tracks[prevBeatID._trackID].beatMidiSliders[prevBeatID._beatID].setVisible(false);
        bars[selectedBeatID._barID].tracks[selectedBeatID._trackID].beatMidiSliders[selectedBeatID._beatID].setVisible(true);
        bars[prevBeatID._barID].tracks[prevBeatID._trackID].beatLabels[prevBeatID._beatID].setVisible(false);
        bars[selectedBeatID._barID].tracks[selectedBeatID._trackID].beatLabels[selectedBeatID._beatID].setVisible(true);
        prevBeatID._barID = selectedBeatID._barID;
        prevBeatID._trackID = selectedBeatID._trackID;
        prevBeatID._beatID = selectedBeatID._beatID;
    }
}

void PolyGnomeAudioProcessorEditor::paintPolyRhythmMachine(juce::Graphics& g) {

    //TODO look into only calling redraws of specific elements if needed
    auto visualArea = getVisualArea();

    //for debugging purposes
    //g.setColour(juce::Colours::white);
    //g.drawRect(visualArea);

    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y



    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    int spacing = height / MAX_TRACKS;
    Y = Y + spacing + 15;
  


    int numBars = audioProcessor.apvts.getRawParameterValue("NUM_BARS")->load();
    int activeBar = audioProcessor.apvts.getRawParameterValue("ACTIVE_BAR")->load();
    int selectedBar = audioProcessor.apvts.getRawParameterValue("SELECTED_BAR")->load();
    bool isProccessorOn = (audioProcessor.apvts.getRawParameterValue("ON/OFF")->load() == true);
    //hide any hidden components
    for (int j = 0; j < MAX_BARS; j++) {
        if (j != selectedBar){
            for (int i = 0; i < MAX_TRACKS; i++) {
                for (auto* comp : getTrackComps(j, i)) {
                    comp->setVisible(false);
                }
            }
        }
    }





    //main loop to draw the tracks and all of its components
    for (int i = 0; i < MAX_TRACKS; i++) {

        //draw the components to the left of the tracks
            //TODO: make X-50, X-125, Y+spacing, etc all constants
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
        bool isBeatEnabled = (isTrackEnabled && (isBarEnabled || !isProccessorOn));
        int subdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(selectedBar, i))->load();
        //draw the buttons for each note of the track
        for (int j = 0; j < subdivisions; j++) {
            //TODO: make this text fit the button properly
            bars[selectedBar].tracks[i].beatButtons[j].setButtonText(juce::String(midiIntToString((audioProcessor.apvts.getRawParameterValue(getBeatMidiString(selectedBar, i, j))->load()))).retainCharacters("ABCDEFG#"));

            //TODO: probably should be in the constructor
            juce::Rectangle<int> beatMidiSliderBounds(X, height, 300, 300);
            bars[selectedBar].tracks[i].beatMidiSliders[j].setBounds(beatMidiSliderBounds);
            juce::Rectangle<int> beatLabelBounds(X, height - 50, 300, 300);
            bars[selectedBar].tracks[i].beatLabels[j].setBounds(beatLabelBounds);

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            bars[selectedBar].tracks[i].beatButtons[j].setBounds(pointBounds);
            bars[selectedBar].tracks[i].beatButtons[j].setVisible(true);

            //TODO : clean this?
            //set the colors of the buttons based on their state
            if (audioProcessor.apvts.getRawParameterValue(getBeatToggleString(selectedBar, i, j))->load() == true) {
                if (j == audioProcessor.polyRhythmMachine.bars[selectedBar].tracks[i].beatCounter - 1) {
                    if (isBeatEnabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR.brighter(0.9));
                    }
                }
                else {
                    if (isBeatEnabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR.brighter(0.9));
                    }
                }
            }
            else {
                if (j == audioProcessor.polyRhythmMachine.bars[selectedBar].tracks[i].beatCounter - 1 && audioProcessor.polyRhythmMachine.bars[selectedBar].tracks[i].subdivisions != 1) {
                    if (isBeatEnabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR.brighter(0.9));
                    }

                }
                else {
                    if (isBeatEnabled) {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                    }
                    else {
                        bars[selectedBar].tracks[i].beatButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR.brighter(0.9));
                    }

                }
            }
        }
        //draw the components to the right of the tracks
        //TODO: make X+10, X+95, X+170, Y+spacing into constants
        juce::Rectangle<int> subdivisionSliderBounds(X + width + 10, Y + spacing * (i - 1), 75, 75);
        bars[selectedBar].tracks[i].subdivisionSlider.setBounds(subdivisionSliderBounds);
        bars[selectedBar].tracks[i].subdivisionSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, spacing / 2);
        bars[selectedBar].tracks[i].subdivisionSlider.setVisible(true);

        juce::Rectangle<int> midiTextEditorBounds(X + width + 10 + 85, Y + spacing * (i - 1) + 25, 75, 25);
        bars[selectedBar].tracks[i].midiTextEditor.setBounds(midiTextEditorBounds);
        bars[selectedBar].tracks[i].midiTextEditor.setVisible(true);
        bars[selectedBar].tracks[i].midiTextEditor.setSelectAllWhenFocused(true);

        juce::Rectangle<int> midiSliderBounds(X + width + 10 + 170, Y + spacing * (i - 1) + 13, 50, 50);
        bars[selectedBar].tracks[i].midiSlider.setBounds(midiSliderBounds);
        bars[selectedBar].tracks[i].midiSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        bars[selectedBar].tracks[i].midiSlider.setVisible(true);

        //adjust the colors of any components belonging to tracks
        colorSlider(bars[selectedBar].tracks[i].subdivisionSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(bars[selectedBar].tracks[i].velocitySlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(bars[selectedBar].tracks[i].sustainSlider, ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorTextEditor(bars[selectedBar].tracks[i].midiTextEditor, ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, isTrackEnabled);



        //hide any hidden track components 
        for (int k = 0; k < MAX_SUBDIVISIONS; k++) {
            if (k >= subdivisions) {
                bars[selectedBar].tracks[i].beatButtons[k].setVisible(false);
            }
        }
    }

}

juce::Rectangle<int> PolyGnomeAudioProcessorEditor::getVisualArea()
{
    auto bounds = getLocalBounds();
    //visual area consists of middle third of top third of area 
    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    visualArea.removeFromLeft(visualArea.getWidth() * 0.33);
    visualArea.removeFromRight(visualArea.getWidth() * 0.5);
    visualArea.setWidth(432); // common multiple of 9, 12, 16 
    return visualArea;
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
    comps.push_back(&menu.playButton);
    comps.push_back(&menu.loadPresetButton);
    comps.push_back(&menu.savePresetButton);
    comps.push_back(&menu.reminderTextEditor);
    comps.push_back(&menu.barSlider);
    comps.push_back(&menu.autoLoopButton);
    comps.push_back(&keyboard);
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
                comps.push_back(&bars[k].tracks[i].beatMidiSliders[j]);
                comps.push_back(&bars[k].tracks[i].beatLabels[j]);
            }
        }
        comps.push_back(&menu.barSelectButtons[k]);
        comps.push_back(&menu.barCopyButtons[k]);
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
    //TODO: make this a class function?
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
    //TODO: this seems incredibly inifficient, probably something in the framework I can find to deal with this
    juce::String   reminderText = "";

    if (menu.playButton.isHoveredOver == true) {
        reminderText = menu.playButton.getHelpText();
    }
    else  if (menu.loadPresetButton.isHoveredOver == true) {
        reminderText = menu.loadPresetButton.getHelpText();
    }
    else  if (menu.savePresetButton.isHoveredOver == true) {
        reminderText = menu.savePresetButton.getHelpText();
    }
    else if (menu.barSlider.isHoveredOver == true) {
        reminderText = menu.barSlider.getHelpText();
    }
    else if (menu.autoLoopButton.isHoveredOver == true) {
        reminderText = menu.autoLoopButton.getHelpText();
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
                        if (bars[j].tracks[i].beatMidiSliders[k].isHoveredOver == true) {
                            reminderText = bars[j].tracks[i].beatMidiSliders[k].getHelpText();
                            break;
                        }

                    }
                }
            }
            if (menu.barSelectButtons[j].isHoveredOver == true) {
                reminderText = menu.barSelectButtons[j].getHelpText();
                break;
            }
            if (menu.barCopyButtons[j].isHoveredOver == true) {
                reminderText = menu.barCopyButtons[j].getHelpText();
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

