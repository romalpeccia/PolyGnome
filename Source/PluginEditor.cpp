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
        play();
    };    
    polyRModeButton.onClick = [this]() { 
        audioProcessor.apvts.getRawParameterValue("MODE")->store(1);
        play();
    };    
    polyMModeButton.onClick = [this]() { 
        audioProcessor.apvts.getRawParameterValue("MODE")->store(2);
        play();
    };

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }



    startTimerHz(144);
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
       paintPolyRMode(g);
        
    }
    else if (mode == 2) {
      //  paintDefaultMode(g);
        ;
    }

}

juce::Rectangle<int> MetroGnomeAudioProcessorEditor::getVisualArea()
{
    auto bounds = getLocalBounds();
    //visual area consists of middle third of top third of area (9 quadrants)
    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    visualArea.removeFromLeft(visualArea.getWidth() * 0.33);
    visualArea.removeFromRight(visualArea.getWidth() * 0.5);
    return visualArea;
}

void MetroGnomeAudioProcessorEditor::paintPolyRMode(juce::Graphics& g)
{
    auto visualArea = getVisualArea();
    //uncomment for debugging purposes    
    g.setColour(juce::Colours::white);
    //g.drawRect(visualArea);

    int width = visualArea.getWidth();
    int height = visualArea.getHeight();
    int circleSkew = 1.2; //TODO adjust this using JUCE_LIVE_CONSTANT
    int radius = ((width > height) ? height : width)/circleSkew;
    //take the lesser of height and width to be our circle diameter
    int X = visualArea.getX(); //top left corner X
    int Y = visualArea.getY(); //top left corner  Y





    //TODO maybe? make seperate sliders and variables instead of reusing the ones from the metronome
    int innerBeats = audioProcessor.apvts.getRawParameterValue("SUBDIVISION")->load();
    int outerBeats = audioProcessor.apvts.getRawParameterValue("NUMERATOR")->load();
    auto ON = audioProcessor.apvts.getRawParameterValue("ON/OFF")->load();
    int pointDiameter = 10;
    //TODO: make these inner and outer beat logic into functions, one for each drawing, and then one for drawing the entire object?
    if (outerBeats!= 1)
    {
        //draw the circle
        juce::Path outerCircle;
        int outerRadius = radius / 1; //TODO maybe change this number, make a param?
        int Xoffset = (width -  outerRadius)/2;
        int Yoffset = (height -  outerRadius)/2;
        outerCircle.addEllipse(X + Xoffset, Y + Yoffset, outerRadius, outerRadius);
        g.setColour(juce::Colours::white);
        g.strokePath(outerCircle, juce::PathStrokeType(2.0f));

        //iterate through points and draw a circle on the edge of the main circle
        float outerLength = outerCircle.getLength();
        for (float i = 0; i < outerBeats; i++)
        {
            g.setColour(juce::Colours::orange);
            float temp = (i / outerBeats) * outerLength;
            auto point = outerCircle.getPointAlongPath(temp );
            g.fillEllipse(point.getX(), point.getY(), pointDiameter, pointDiameter);
        }

        //draw the clock hand
        float angle = juce::degreesToRadians(360 * (float(audioProcessor.polyRmetronome.getRhythm1Counter()) / float(outerBeats)) - 180);
            //*(audioProcessor.polyRmetronome.getSamplesProcessed() / audioProcessor.getSampleRate()));
        //angle = juce::jmap(angle, 0.f, 1.f, juce::degreesToRadians(0), juce::degreesToRadians(360));

        juce::Point<int> center(X + outerRadius/2, Y + (height - Xoffset) / 2);
        //radius is just outerRadius from before
        juce::Path p;
        juce::Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(center.getY());
        r.setBottom(center.getY() + outerRadius/2);
        p.addRoundedRectangle(r, 2.f);
        p.applyTransform(juce::AffineTransform().rotated(angle, center.getX(), center.getY()));
        g.fillPath(p);

        DBG(juce::radiansToDegrees(angle));
        g.drawText(juce::String(audioProcessor.polyRmetronome.getRhythm1Counter()), 5, 5, 310, 310, juce::Justification::centred);
    }
    if (innerBeats != 1)
    {
        //draw the circle
        juce::Path innerCircle;
        int innerRadius = radius / 1.5;
        int Xoffset = (width - innerRadius) / 2;
        int Yoffset = (height - innerRadius) / 2;
        innerCircle.addEllipse(X + Xoffset, Y + Yoffset, innerRadius, innerRadius);
        g.setColour(juce::Colours::orange);
        g.strokePath(innerCircle, juce::PathStrokeType(2.0f));

        //iterate through points and draw a circle on the edge of the main circle
        float innerLength = innerCircle.getLength();
        for (float i = 0; i < innerBeats; i++)
        {
            g.setColour(juce::Colours::white);
            float temp = (i / innerBeats) * innerLength;
            auto point = innerCircle.getPointAlongPath(temp);
            g.fillEllipse(point.getX(), point.getY(), pointDiameter, pointDiameter);
        }

        //draw the clock hand
        float angle = juce::degreesToRadians(360 * (float(audioProcessor.polyRmetronome.getRhythm2Counter()) / float(innerBeats)) -180 );
        //*(audioProcessor.polyRmetronome.getSamplesProcessed() / audioProcessor.getSampleRate()));
    //angle = juce::jmap(angle, 0.f, 1.f, juce::degreesToRadians(0), juce::degreesToRadians(360));
        


        juce::Point<int> center(X + Xoffset + innerRadius/2, Y + Yoffset + innerRadius / 2);
        //radius is just innerRadius from before
        juce::Path p;
        juce::Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(center.getY());
        r.setBottom(center.getY() + innerRadius / 2);
        p.addRoundedRectangle(r, 2.f);
        p.applyTransform(juce::AffineTransform().rotated(angle, center.getX(), center.getY()));
        g.fillPath(p);

        //DBG(juce::radiansToDegrees(angle));
        g.drawText(juce::String(audioProcessor.polyRmetronome.getRhythm2Counter()), 5, 5, 300,300, juce::Justification::centred);

    }
}



void MetroGnomeAudioProcessorEditor::paintDefaultMode(juce::Graphics& g) {


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
    X = visualArea.getX();
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
    //flexBox.items.add(juce::FlexItem(150, 50, placeholderButton));
    flexBox.performLayout(playBounds);



    auto visualArea = bounds.removeFromTop(bounds.getHeight() * 0.66);
    //unused variable but cuts away space used by the visual area from bounds, may be used later


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
    audioProcessor.polyRmetronome.resetall();
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