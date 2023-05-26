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


    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);

    //initialize the polyrhythm Machine buttons and sliders
    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++)
        { //initialize the track buttons
            juce::String name = "BEAT_" + to_string(i) + "_" + to_string(j) + "_TOGGLE";
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
        polyRhythmMachineMuteButtonAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "TRACK_" + to_string(i) + "_ENABLE", polyRhythmMachineMuteButtons[i]);


        //initialize the subdivision sliders
        polyRhythmMachineSubdivisionSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::orange);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::orange);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colours::steelblue);
        polyRhythmMachineSubdivisionSliders[i].setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colours::orange);

        polyRhythmMachineSubdivisionSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "SUBDIVISIONS_" + to_string(i),
            polyRhythmMachineSubdivisionSliders[i]);
         

        //initialize the MIDI control slider    
        polyRhythmMachineMidiSliders[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
        polyRhythmMachineMidiSliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts,
            "MIDI_VALUE_" + to_string(i),
            polyRhythmMachineMidiSliders[i]);

        //initialize the text entry logic and UI for MIDI Values
        int currentIntValue = audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
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
            int currentIntValue = audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->load();
            if (sscanf(inputString.c_str(), "%d", &inputInt) == 1)
            {   //if the user inputted an int
                convertedString = midiIntToString(inputInt);
                if (convertedString != "") {
                    polyRhythmMachineMidiTextEditors[i].setText(convertedString + " / " + to_string(inputInt));
                    audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->store(inputInt);
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
                    audioProcessor.apvts.getRawParameterValue("MIDI_VALUE_" + to_string(i))->store(convertedInt);
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
    g.fillAll(juce::Colours::black);
    g.drawImageAt(logo, 0, 0);


    if (audioProcessor.apvts.getRawParameterValue("HOST_CONNECTED")->load()){
        bpmSlider.setEnabled(false);
    }
        paintPolyRhythmMachineMode(g);
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

void PolyGnomeAudioProcessorEditor::paintPolyRhythmMachineMode(juce::Graphics& g) {

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
        bool trackEnabled = audioProcessor.apvts.getRawParameterValue("TRACK_" + to_string(i) + "_ENABLE")->load();

        //draw the buttons for each note of the track
        int subdivisions = audioProcessor.apvts.getRawParameterValue("SUBDIVISIONS_" + to_string(i))->load();
        for (int j = 0; j < subdivisions; j++) {

            float distanceOnPath = (width / subdivisions) * j;
            juce::Rectangle<int> pointBounds(X + distanceOnPath, Y + spacing * i - 10, 22, 22);
            polyRhythmMachineButtons[i][j].setBounds(pointBounds);
            polyRhythmMachineButtons[i][j].setVisible(true);


            //TODO : clean this?
            if (audioProcessor.apvts.getRawParameterValue("BEAT_" + to_string(i) + "_" + to_string(j) + "_TOGGLE")->load() == true) {
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
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::steelblue);
}
void PolyGnomeAudioProcessorEditor::togglePlayStateOn() {
    audioProcessor.apvts.getRawParameterValue("ON/OFF")->store(true);
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indigo);
}
std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getVisibleComps() {


    std::vector<juce::Component*> comps;
    comps.push_back(&playButton);
    comps.push_back(&bpmSlider);

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        comps.push_back(&polyRhythmMachineSubdivisionSliders[i]);
        comps.push_back(&polyRhythmMachineMidiSliders[i]);
        comps.push_back(&polyRhythmMachineMidiTextEditors[i]);
        comps.push_back(&polyRhythmMachineMuteButtons[i]);
    }

    return{ comps };
}

std::vector<juce::Component*> PolyGnomeAudioProcessorEditor::getHiddenComps() {

    std::vector<juce::Component*> comps;
    comps.push_back(&loadPresetButton);
    comps.push_back(&savePresetButton);

    for (int i = 0; i < MAX_MIDI_CHANNELS; i++) {
        for (int j = 0; j < MAX_LENGTH; j++) {
            comps.push_back(&polyRhythmMachineButtons[i][j]);
        }
    }

    return{ comps };
}

void PolyGnomeAudioProcessorEditor::changeMenuButtonColors(juce::TextButton *buttonOn) {
    auto buttonColourId = juce::TextButton::ColourIds::buttonColourId;
    buttonOn->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::indigo);
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