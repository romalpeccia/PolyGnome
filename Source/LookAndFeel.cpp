/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 6 Dec 2021 10:46:51am
    Author:  Romal

  ==============================================================================
*/

#include "LookAndFeel.h"




void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle,
    float rotaryEndAngle, juce::Slider& slider)
{
    using namespace juce;
    auto bounds = Rectangle<float>(x, y, width, height);
    auto enabled = slider.isEnabled();

    if (enabled)
    {
        g.setColour(Colour(48u, 9u, 84));
        g.fillEllipse(bounds);

        g.setColour(Colour(128u, 77u, 1u));
        g.drawEllipse(bounds, 1.f);
    }
    else
    {
        g.setColour(Colours::darkgrey);
        g.fillEllipse(bounds);

        g.setColour(Colours::black);
        g.drawEllipse(bounds, 1.f);
    }
    //TODO: if we can cast rswl to rotaryslider with labels then we can use its functions??? why is this needed
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider)) {

        //make main rectangle
        auto center = bounds.getCentre();

        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5); // subtract text height
        p.addRoundedRectangle(r, 2.f);
        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        g.fillPath(p);

        // make value text rectangle
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(center);
        g.setColour(enabled ? Colour(48u, 9u, 84) : Colours::darkgrey);
        g.fillRect(r);
        g.setColour(enabled ? Colours::orange : Colours::darkgrey);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}


//==============================================================================
void RotarySliderWithLabels::paint(juce::Graphics& g) {
    //NOTE: the appearance of any other Component is determined by the implementation of its paint() function.


    using namespace juce;
    auto startAngle = degreesToRadians(180.f + 45.f);
    auto endAngle = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi; // add two pi because the range needs to go from low to high
    auto range = getRange();
    auto sliderBounds = getSliderBounds();

    //bounding boxes to help debug/visualize
    /*
    g.setColour(Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);
    */
    getLookAndFeel().drawRotarySlider(g, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAngle, endAngle, *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::white);
    g.setFont(getTextHeight());

    //iterate through labels and draw them?
    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto ang = jmap(pos, 0.f, 1.f, startAngle, endAngle);
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }

}


juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const {
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    //make space for text?
    size -= getTextHeight() * 2;
    //make a square so our ellipse is a circle
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
// returns the value of the slider param as a string
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();
    juce::String str;


    //failsafe check to see what type of param we are using
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float value = getValue();
        str = juce::String(value);
    }
    else if (auto* intParam = dynamic_cast<juce::AudioParameterInt*>(param))
    {
        int value = getValue();
            str = juce::String(value);
    }
    else

    {
        jassertfalse; // this shouldn't happen in this project unless I add new params
    }

    if (suffix.isNotEmpty()) {
        str << " ";
        str << suffix;
    }


    return str;
}
//==============================================================================