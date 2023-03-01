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

    RotarySliderWithLabels() {
       
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





