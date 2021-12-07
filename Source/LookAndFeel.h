/*
  ==============================================================================

    LookAndFeel.h
    Created: 6 Dec 2021 10:46:51am
    Author:  Romal

  ==============================================================================
*/
#include <JuceHeader.h>
#pragma once

struct CustomLookAndFeel : juce::LookAndFeel_V4 {
    //put all custom aesthetic functions herE?
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
        float sliderPosProportional, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider&) override;

    /*
    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;*/

};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) :
        juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox),
        param(&rap),
        suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }

    ~RotarySliderWithLabels() {
        setLookAndFeel(nullptr);
    }

    struct LabelPos
    {
        float pos;
        juce::String label;
    };

    juce::Array<LabelPos> labels;


    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return textHeight; }
    juce::String getDisplayString() const;


private:
    CustomLookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
    int textHeight = 14;
};

/*
struct TimeVisualizerComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    TimeVisualizerComponent(MetroGnomeAudioProcessor&);
    ~TimeVisualizerComponent();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    void timerCallback() override;
    void paint(juce::Graphics& g) override;

    void resized() override;


private:
    MetroGnomeAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

    //make it an image since it doesnt need to be redrawn
    juce::Image background;

    //overall response curve GUI area
    juce::Rectangle<int> getRenderArea();
    //response curve area
    juce::Rectangle<int> getAnalysisArea();

};


void TimeVisualizerComponent::paint(juce::Graphics& g)
{

    using namespace juce;
    g.fillAll(Colours::black);

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
    //end making visualizer

}

void TimeVisualizerComponent::resized()
{

}


juce::Rectangle<int> TimeVisualizerComponent::getRenderArea()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> TimeVisualizerComponent::getAnalysisArea()
{
*/