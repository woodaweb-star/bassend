#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

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

    struct KnobBlock
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    KnobBlock knobs[4];
    float displayedReduction = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassendEditor)
};
