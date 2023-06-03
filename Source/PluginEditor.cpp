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

    //initialize the rack options
    rackSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    colorSlider(rackSlider, MAIN_COLOUR, MAIN_COLOUR, SECONDARY_COLOUR, MAIN_COLOUR, true);
    rackSliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NUM_RACKS", rackSlider);
    rackSlider.setHelpText(RACK_SLIDER_REMINDER);

    for (int i = 0; i < MAX_RACKS; i++)
    { //initialize the track buttons
        juce::String name = "SELECTED_RACK";
        rackButtons[i].onClick = [this, name, i]() {
            audioProcessor.apvts.getRawParameterValue(name)->store(i);
            for (int j = 0; j < MAX_RACKS; j++) {
                rackButtons[j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
            }
            rackButtons[i].setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
        };
        rackButtons[i].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
        rackButtons[i].setHelpText(RACK_BUTTON_REMINDER);
    }


    //initialize the polytrack Machine buttons and sliders
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_TRACK_LENGTH; j++)
        { //initialize the track buttons
            juce::String name = getBeatToggleString(i, j);
            beatButtons[i][j].onClick = [this, name, i, j]() {
                if (audioProcessor.apvts.getRawParameterValue(name)->load() == true) {
                    beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                }
                else {
                    beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                }
            };
            beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
            beatButtons[i][j].setClickingTogglesState(true); //what does this do?
            beatButtonAttachments[i][j] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, name,beatButtons[i][j]);
            beatButtons[i][j].setHelpText(BEAT_BUTTON_REMINDER);
        }

        //initialize the mute buttons
        muteButtons[i].setClickingTogglesState(true);
        muteButtonAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, getTrackEnableString(i), muteButtons[i]);
        muteButtons[i].setHelpText(MUTE_BUTTON_REMINDER);

        //initialize the subdivision sliders
        subdivisionSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        colorSlider(subdivisionSliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
        subdivisionSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getSubdivisionsString(i), subdivisionSliders[i]);
        subdivisionSliders[i].setHelpText(SUBDIVISION_SLIDER_REMINDER);

        //initialize the velocity sliders
        velocitySliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        colorSlider(velocitySliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
        velocitySliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getVelocityString(i), velocitySliders[i]);
        velocitySliders[i].setHelpText(VELOCITY_SLIDER_REMINDER);

        //initialize the sustain sliders
        colorSlider(sustainSliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, true);
        sustainSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getSustainString(i),sustainSliders[i]);
        sustainSliders[i].setHelpText(SUSTAIN_SLIDER_REMINDER);


        //initialize the MIDI control slider    
        midiSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        midiSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, getMidiValueString(i), midiSliders[i]);
        midiSliders[i].setHelpText(MIDI_SLIDER_REMINDER);

        //initialize the text entry logic and UI for MIDI Values
        int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(i))->load();
        midiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
        midiTextEditors[i].setReadOnly(false);
        colorTextEditor(midiTextEditors[i], ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, true);
        midiTextEditors[i].setHelpText(MIDI_TEXTEDITOR_REMINDER);


        midiTextEditors[i].onReturnKey = [this, i]() {
            juce::String input = midiTextEditors[i].getText();
            string inputString = input.toStdString();
            int inputInt;
            string convertedString;
            int currentIntValue = audioProcessor.apvts.getRawParameterValue(getMidiValueString(i))->load();
            if (sscanf(inputString.c_str(), "%d", &inputInt) == 1)
            {   //if the user inputted an int
                convertedString = midiIntToString(inputInt);
                if (convertedString != "") {
                    midiTextEditors[i].setText(convertedString + " / " + to_string(inputInt));
                    audioProcessor.apvts.getRawParameterValue(getMidiValueString(i))->store(inputInt);
                    //add control of MIDI slider to textbox
                    midiSliders[i].setValue(inputInt);
                }
                else {
                    midiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                }
            } 
            else
            { //user inputted a string
                int convertedInt = midiStringToInt(inputString);
                if (convertedInt != -1) {
                    midiTextEditors[i].setText(inputString + " / " + to_string(convertedInt));
                    audioProcessor.apvts.getRawParameterValue(getMidiValueString(i))->store(convertedInt);
                    //add control of MIDI slider to textbox
                    midiSliders[i].setValue(convertedInt);
                }
                else {
                    midiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
                }
            }   
        };

        //add control of textbox to MIDI slider
        midiSliders[i].onValueChange = [this, i]() {
            int sliderInt = midiSliders[i].getValue();
            midiTextEditors[i].setText(midiIntToString(sliderInt) + " / " + to_string(sliderInt));
        };


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
    flexBox.items.add(juce::FlexItem(100, 200, rackSlider));
    for (int i = 0; i < MAX_RACKS; i++) {
        flexBox.items.add(juce::FlexItem(25, 25, rackButtons[i]));
    }
    flexBox.performLayout(menuBounds);
  
}

PolyGnomeAudioProcessorEditor::~PolyGnomeAudioProcessorEditor()
{
}

//==============================================================================

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
    else if (rackSlider.isHoveredOver == true) {
        reminderText = rackSlider.getHelpText();
    }
    else {
        for (int i = 0; i < MAX_RACKS; i++) {
            if (rackButtons[i].isHoveredOver == true) {
                reminderText = rackButtons[i].getHelpText();
                break;
            }
        }
        for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
            if (muteButtons[i].isHoveredOver == true) {
                reminderText = muteButtons[i].getHelpText();
                break;
            }
            else if (subdivisionSliders[i].isHoveredOver == true) {
                reminderText = subdivisionSliders[i].getHelpText();
                break;
            }
            else if (midiSliders[i].isHoveredOver == true) {
                reminderText = midiSliders[i].getHelpText();
                break;
            }
            else if (midiTextEditors[i].isHoveredOver == true) {
                reminderText = midiTextEditors[i].getHelpText();
                break;
            }
            else if (velocitySliders[i].isHoveredOver == true) {
                reminderText = velocitySliders[i].getHelpText();
                break;
            }
            else if (sustainSliders[i].isHoveredOver == true) {
                reminderText = sustainSliders[i].getHelpText();
                break;
            }
            else {
                for (int j = 0; j < MAX_TRACK_LENGTH; j++) {
                    if (beatButtons[i][j].isHoveredOver == true) {
                        reminderText = beatButtons[i][j].getHelpText();
                        break;
                    }
                }
            }
        }

    }
    return reminderText;
}

void PolyGnomeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BACKGROUND_COLOUR);
    g.drawImageAt(logo, 0, 0);


    int numRacks = audioProcessor.apvts.getRawParameterValue("NUM_RACKS")->load();
    for (int j = 0; j < numRacks; j++) {
        rackButtons[j].setVisible(true);
    }
    for (int j = numRacks; j < MAX_RACKS; j++) {
        rackButtons[j].setVisible(false);
    }

    
    reminderTextEditor.setText(getCurrentMouseOverText());
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

    int spacing = height / MAX_MIDI_CHANNELS;
    Y = Y + spacing;

    //main loop to draw the tracks and all of its components
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {

        //draw the components to the left of the tracks
        juce::Rectangle<int> muteButtonBounds(X - 50, Y + spacing * (i - 1) + 13, 50, 50);
        muteButtons[i].setBounds(muteButtonBounds);
        muteButtons[i].setVisible(true);

        juce::Rectangle<int> velocitySliderBounds(X - 125, Y + spacing * (i - 1), 75, 75);
        velocitySliders[i].setBounds(velocitySliderBounds);
        velocitySliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, 35, spacing / 2);
        velocitySliders[i].setVisible(true);

        juce::Rectangle<int> sustainSliderBounds(X - 200, Y + spacing * (i - 1) + 13, 50, 50);
        sustainSliders[i].setBounds(sustainSliderBounds);
        sustainSliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 35, spacing / 2);
        sustainSliders[i].setVisible(true);


        //draw the line segments representing each track
        juce::Path trackLine;
        trackLine.addLineSegment(juce::Line<float>(X, Y + spacing * i, X + width, Y + spacing * i), 1.f);
        g.setColour(SECONDARY_COLOUR);
        g.strokePath(trackLine, juce::PathStrokeType(2.0f));
        bool isTrackEnabled = audioProcessor.apvts.getRawParameterValue(getTrackEnableString(i))->load();

        //draw the buttons for each note of the track
        int subdivisions = audioProcessor.apvts.getRawParameterValue(getSubdivisionsString(i))->load();
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            beatButtons[i][j].setBounds(pointBounds);
            beatButtons[i][j].setVisible(true);


            //TODO : clean this?
            if (audioProcessor.apvts.getRawParameterValue(getBeatToggleString(i, j))->load() == true) {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1) {
                    if (isTrackEnabled == true) {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR.brighter(0.9));
                    }

                }
                else {
                    if (isTrackEnabled == true){
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR.brighter(0.9));
                    }
                }
            }
            else {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1 && audioProcessor.polyRhythmMachine.tracks[i].subdivisions != 1) {
                    if (isTrackEnabled == true) {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR.brighter(0.9));
                    }
                    
                }
                else {
                    if (isTrackEnabled == true) {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR.brighter(0.9));
                    }
                    
                }
            }
        }
        //hide any hidden buttons
        for (int k = subdivisions; k < MAX_TRACK_LENGTH; k++) {
            beatButtons[i][k].setVisible(false);
        }

        //draw the components to the right of the tracks
        juce::Rectangle<int> subdivisionSliderBounds(X + width + 10, Y + spacing * (i - 1), 75, 75);
        subdivisionSliders[i].setBounds(subdivisionSliderBounds);
        subdivisionSliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxRight, false, 25, spacing / 2);
        subdivisionSliders[i].setVisible(true);

        juce::Rectangle<int> midiTextEditorBounds(X + width + 10 + 85, Y + spacing * (i - 1) + 25, 75, 25);
        midiTextEditors[i].setBounds(midiTextEditorBounds);
        midiTextEditors[i].setVisible(true);

        juce::Rectangle<int> midiSliderBounds(X + width + 10 + 170, Y + spacing * (i - 1) + 13, 50, 50);
        midiSliders[i].setBounds(midiSliderBounds);
        midiSliders[i].setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        midiSliders[i].setVisible(true);

        //adjust the colors of any components belonging to tracks
        colorSlider(subdivisionSliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(velocitySliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorSlider(sustainSliders[i], ACCENT_COLOUR, ACCENT_COLOUR, SECONDARY_COLOUR, ACCENT_COLOUR, isTrackEnabled);
        colorTextEditor(midiTextEditors[i], ACCENT_COLOUR, ACCENT_COLOUR, ACCENT_COLOUR, MAIN_COLOUR, isTrackEnabled);
 
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
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
}
void PolyGnomeAudioProcessorEditor::togglePlayStateOn() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
}


std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getTrackComps(int index) {
    //returns components for track[i]
    std::vector<juce::Component*> comps;
    comps.push_back(&subdivisionSliders[index]);
    comps.push_back(&velocitySliders[index]);
    comps.push_back(&midiSliders[index]);
    comps.push_back(&midiTextEditors[index]);
    comps.push_back(&muteButtons[index]);
    comps.push_back(&sustainSliders[index]);
    for (int j = 0; j < MAX_TRACK_LENGTH; j++) {
        comps.push_back(&beatButtons[index][j]);
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
    comps.push_back(&rackSlider);
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        comps.push_back(&subdivisionSliders[i]);
        comps.push_back(&velocitySliders[i]);
        comps.push_back(&midiSliders[i]);
        comps.push_back(&midiTextEditors[i]);
        comps.push_back(&muteButtons[i]);
        comps.push_back(&sustainSliders[i]);
    }

    return{ comps };
}

std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getHiddenComps() {
    //returns components that sometimes have their state changed to hidden

    std::vector<juce::Component*> comps;
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_TRACK_LENGTH; j++) {
            comps.push_back(&beatButtons[i][j]);
        }
    }
    for (int i = 0; i < MAX_RACKS; i++) {
        comps.push_back(&rackButtons[i]);
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

void PolyGnomeAudioProcessorEditor::changeOptionsButtonColors(juce::TextButton *buttonOn) {
    buttonOn->setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
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