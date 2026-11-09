#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

// Knob con aspecto analógico: anillo de escala, cuerpo metálico, estrías y puntero dorado.
class BassendLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BassendLookAndFeel() = default;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;
};

class BassendEditor : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit BassendEditor (BassendProcessor&);
    ~BassendEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BassendProcessor& processor;

    // El LookAndFeel va antes que el knob para destruirse después.
    BassendLookAndFeel lookAndFeel;
    juce::Slider intensityKnob;
    juce::Label intensityLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::TextButton presetButtons[4];

    float displayedReduction = 0.0f;
    float displayedIntensity = -1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassendEditor)
};
