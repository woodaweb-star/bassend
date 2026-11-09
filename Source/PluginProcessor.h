#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

// NARMIA.AI Bassend: sidechain ducker de propósito general.
// Entrada principal = bajo/pads. Sidechain = kick.
class BassendProcessor : public juce::AudioProcessor
{
public:
    BassendProcessor();
    ~BassendProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "NARMIA.AI Bassend"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Público para que el editor pueda conectar los controles y leer el medidor.
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> reductionMeter { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* depthParam   = nullptr;
    std::atomic<float>* attackParam  = nullptr;
    std::atomic<float>* holdParam    = nullptr;
    std::atomic<float>* releaseParam = nullptr;

    double sampleRate = 44100.0;
    float detectorEnv = 0.0f;
    float currentDuck = 0.0f;
    int holdCounter   = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassendProcessor)
};
