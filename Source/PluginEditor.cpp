#include "PluginEditor.h"

namespace BassendColours
{
    const juce::Colour background { 0xff0a0a0a };
    const juce::Colour panel      { 0xff161616 };
    const juce::Colour track      { 0xff2a2a2a };
    const juce::Colour green      { 0xff3fc46b };
    const juce::Colour gold       { 0xffd4af37 };
    const juce::Colour grey       { 0xff8a8a8a };
    const juce::Colour white      { 0xfff2f2f2 };
}

BassendEditor::BassendEditor (BassendProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (560, 320);

    const char* paramIds[] = { "depth", "attack", "hold", "release" };
    const char* names[]    = { "DEPTH", "ATTACK", "HOLD", "RELEASE" };

    for (int i = 0; i < 4; ++i)
    {
        auto& k = knobs[i];

        k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        k.slider.setColour (juce::Slider::rotarySliderFillColourId, BassendColours::green);
        k.slider.setColour (juce::Slider::rotarySliderOutlineColourId, BassendColours::track);
        k.slider.setColour (juce::Slider::thumbColourId, BassendColours::gold);
        k.slider.setColour (juce::Slider::textBoxTextColourId, BassendColours::white);
        k.slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        k.slider.setColour (juce::Slider::textBoxBackgroundColourId, BassendColours::panel);
        addAndMakeVisible (k.slider);

        k.label.setText (names[i], juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, BassendColours::gold);
        addAndMakeVisible (k.label);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, paramIds[i], k.slider);
    }

    startTimerHz (30);
}

BassendEditor::~BassendEditor()
{
    stopTimer();
}

void BassendEditor::paint (juce::Graphics& g)
{
    g.fillAll (BassendColours::background);

    // Cabecera
    auto header = getLocalBounds().removeFromTop (56).toFloat();
    g.setColour (BassendColours::gold);
    g.drawText ("NARMIA.AI", header.reduced (20.0f, 0.0f), juce::Justification::centredLeft, false);
    g.setColour (BassendColours::white);
    g.drawText ("BASSEND", header.reduced (20.0f, 0.0f), juce::Justification::centredRight, false);

    // Medidor de reducción de ganancia
    auto meter = getLocalBounds().removeFromBottom (40).reduced (20, 10).toFloat();
    g.setColour (BassendColours::panel);
    g.fillRoundedRectangle (meter, 4.0f);

    const float fillWidth = meter.getWidth() * juce::jlimit (0.0f, 1.0f, displayedReduction);
    g.setColour (BassendColours::green);
    g.fillRoundedRectangle (meter.withWidth (fillWidth), 4.0f);

    g.setColour (BassendColours::grey);
    g.drawText ("GAIN REDUCTION", meter, juce::Justification::centred, false);
}

void BassendEditor::resized()
{
    auto area = getLocalBounds().reduced (16, 0);
    area.removeFromTop (56);
    area.removeFromBottom (54);

    const int cellWidth = area.getWidth() / 4;

    for (auto& k : knobs)
    {
        auto cell = area.removeFromLeft (cellWidth).reduced (6, 8);
        k.label.setBounds (cell.removeFromTop (20));
        k.slider.setBounds (cell);
    }
}

void BassendEditor::timerCallback()
{
    const float value = processor.reductionMeter.load();

    if (std::abs (value - displayedReduction) > 0.001f)
    {
        displayedReduction = value;
        repaint (getLocalBounds().removeFromBottom (54));
    }
}
