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
    : AudioProcessorEditor(&p), audioProcessor(p),
    bpmSlider(*audioProcessor.apvts.getParameter("BPM"), "bpm"),
    bpmAttachment(audioProcessor.apvts, "BPM", bpmSlider)
{
    //images are stored in binary using projucer
    logo = juce::ImageCache::getFromMemory(BinaryData::OSRS_gnome_png, BinaryData::OSRS_gnome_pngSize);

    //initialize the menu buttons
    playButton.onClick = [this]() { 

        togglePlayState();
        toggleAudioProcessorChildrenStates();
    };

    loadPresetButton.onClick = [this]() {
        loadPreset();
    };
    savePresetButton.onClick = [this]() {
        savePreset();
    };


    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);

    //initialize the polytrack Machine buttons and sliders
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_TRACK_LENGTH; j++)
        { //initialize the track buttons
            juce::String name = "BEAT_" + to_string(i) + "_" + to_string(j) + "_TOGGLE";
            beatButtons[i][j].onClick = [this, name, i, j]() {
                if (audioProcessor.apvts.getRawParameterValue(name)->load() == true) {
                    //audioProcessor.apvts.getRawParameterValue(name)->store(false);
                    beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
                }
                else {
                    //audioProcessor.apvts.getRawParameterValue(name)->store(true);
                    beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                }
            };
            beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_COLOUR);
            beatButtons[i][j].setClickingTogglesState(true);
            beatButtonAttachments[i][j] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, name,beatButtons[i][j]);
        }

        //initialize the mute buttons
        muteButtons[i].setClickingTogglesState(true);
        muteButtonAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "TRACK_" + to_string(i) + "_ENABLE", muteButtons[i]);


        //initialize the subdivision sliders
        subdivisionSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        subdivisionSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
        subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
        subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
        subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);
        subdivisionSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SUBDIVISIONS_" + to_string(i), subdivisionSliders[i]);
         

        //initialize the velocity sliders
        velocitySliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        velocitySliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
        velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
        velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
        velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);
        velocitySliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"VELOCITY_" + to_string(i), velocitySliders[i]);

        //initialize the sustain sliders
        sustainSliders[i].setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        sustainSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
        sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
        sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
        sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);
        sustainSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"SUSTAIN_" + to_string(i),sustainSliders[i]);

        //initialize the MIDI control slider    
        midiSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        midiSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,"MIDI_VALUE_" + to_string(i), midiSliders[i]);

        //initialize the text entry logic and UI for MIDI Values
        int currentIntValue = audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
        midiTextEditors[i].setText(midiIntToString(currentIntValue) + " / " + to_string(currentIntValue));
        midiTextEditors[i].setReadOnly(false);
        midiTextEditors[i].setColour(juce::TextEditor::ColourIds::textColourId, ACCENT_COLOUR);
        midiTextEditors[i].setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, ACCENT_COLOUR);
        midiTextEditors[i].setColour(juce::TextEditor::ColourIds::outlineColourId, ACCENT_COLOUR);
        midiTextEditors[i].setColour(juce::TextEditor::ColourIds::backgroundColourId, MAIN_COLOUR);
        midiTextEditors[i].onReturnKey = [this, i]() {
            juce::String input = midiTextEditors[i].getText();
            string inputString = input.toStdString();
            int inputInt;
            string convertedString;
            int currentIntValue = audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
            if (sscanf(inputString.c_str(), "%d", &inputInt) == 1)
            {   //if the user inputted an int
                convertedString = midiIntToString(inputInt);
                if (convertedString != "") {
                    midiTextEditors[i].setText(convertedString + " / " + to_string(inputInt));
                    audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->store(inputInt);
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
                    audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->store(convertedInt);
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

    juce::Rectangle<int> bounds = getLocalBounds();
    juce::Rectangle<int> playBounds(100, 100);
    playBounds.removeFromTop(50);
    playBounds.removeFromRight(50);

    juce::FlexBox flexBox;
    flexBox.flexWrap = juce::FlexBox::Wrap::wrap;
    flexBox.items.add(juce::FlexItem(50, 50, playButton));
    flexBox.items.add(juce::FlexItem(75, 50, loadPresetButton));
    flexBox.items.add(juce::FlexItem(100, 50, savePresetButton));
    flexBox.performLayout(playBounds);



    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    //unused variable, this call cuts away space used by the visual area from bounds, may be used later


    bounds.removeFromTop(5);
    //every time we remove from bounds, the area of bounds changes 
    //first we remove 1/3 * 1 unit area, then we remove 0.5 * 2/3rd unit area
    auto leftArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto rightArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    bpmSlider.setBounds(leftArea);
}

PolyGnomeAudioProcessorEditor::~PolyGnomeAudioProcessorEditor()
{
}

//==============================================================================
void PolyGnomeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BACKGROUND_COLOUR);
    g.drawImageAt(logo, 0, 0);


    if (audioProcessor.apvts.getRawParameterValue("HOST_CONNECTED")->load()){
        bpmSlider.setEnabled(false);
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

    loadPresetButton.setVisible(true);
    savePresetButton.setVisible(true);
    //TODO look into only calling redraws of specific elements if needed
    auto visualArea = getVisualArea();


    g.drawRect(visualArea);

    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y

    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();

    int spacing = height / MAX_MIDI_CHANNELS;
    Y = Y + spacing;





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
        bool trackEnabled = audioProcessor.apvts.getRawParameterValue("TRACK_" + to_string(i) + "_ENABLE")->load();

        //draw the buttons for each note of the track
        int subdivisions = audioProcessor.apvts.getRawParameterValue("SUBDIVISIONS_" + to_string(i))->load();
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            beatButtons[i][j].setBounds(pointBounds);
            beatButtons[i][j].setVisible(true);


            //TODO : clean this?
            if (audioProcessor.apvts.getRawParameterValue("BEAT_" + to_string(i) + "_" + to_string(j) + "_TOGGLE")->load() == true) {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1) {
                    if (trackEnabled == true) {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, ENABLED_COLOUR.brighter(0.9));
                    }

                }
                else {
                    if (trackEnabled == true){
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonOnColourId, MAIN_COLOUR.brighter(0.9));
                    }
                }
            }
            else {
                if (j == audioProcessor.polyRhythmMachine.tracks[i].beatCounter - 1 && audioProcessor.polyRhythmMachine.tracks[i].subdivisions != 1) {
                    if (trackEnabled == true) {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR);
                    }
                    else {
                        beatButtons[i][j].setColour(juce::TextButton::ColourIds::buttonColourId, DISABLED_DARK_COLOUR.brighter(0.9));
                    }
                    
                }
                else {
                    if (trackEnabled == true) {
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

        //adjust the colors of any components belonging to tracks that have been muted

        //TODO: make colouring these into a function since you do it multiple times in the plugineditor
        //one for texteditor, one for either slider
        if (trackEnabled == false) {

            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::textColourId, ACCENT_COLOUR.brighter(0.9));
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, ACCENT_COLOUR.brighter(0.9));
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::outlineColourId, ACCENT_COLOUR.brighter(0.9));
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::backgroundColourId, MAIN_COLOUR.brighter(0.9));

            subdivisionSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR.brighter(0.9));
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR.brighter(0.9));
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR.brighter(0.9));
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR.brighter(0.9));

            velocitySliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR.brighter(0.9));
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR.brighter(0.9));
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR.brighter(0.9));
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR.brighter(0.9));

            sustainSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR.brighter(0.9));
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR.brighter(0.9));
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR.brighter(0.9));
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR.brighter(0.9));
        }
        else {
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::textColourId, ACCENT_COLOUR);
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, ACCENT_COLOUR);
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::outlineColourId, ACCENT_COLOUR);
            midiTextEditors[i].setColour(juce::TextEditor::ColourIds::backgroundColourId, MAIN_COLOUR);

            subdivisionSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
            subdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);

            velocitySliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
            velocitySliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);

            sustainSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, ACCENT_COLOUR);
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, ACCENT_COLOUR);
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, SECONDARY_COLOUR);
            sustainSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, ACCENT_COLOUR);
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
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, SECONDARY_COLOUR);
}
void PolyGnomeAudioProcessorEditor::togglePlayStateOn() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, MAIN_COLOUR);
}
std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getVisibleComps() {
    //returns components that do not need to be hidden

    std::vector<juce::Component*> comps;
    comps.push_back(&playButton);
    comps.push_back(&bpmSlider);
    comps.push_back(&loadPresetButton);
    comps.push_back(&savePresetButton);

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


std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getHiddenComps() {
    //returns components that sometimes have their state changed to hidden

    std::vector<juce::Component*> comps;
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_TRACK_LENGTH; j++) {
            comps.push_back(&beatButtons[i][j]);
        }
    }

    return{ comps };
}

void PolyGnomeAudioProcessorEditor::changeMenuButtonColors(juce::TextButton *buttonOn) {
    auto buttonColourId = juce::TextButton::ColourIds::buttonColourId;
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