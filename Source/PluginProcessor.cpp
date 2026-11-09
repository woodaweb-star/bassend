#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Coeficiente de un filtro de un polo para un tiempo en milisegundos.
    float coefFromMs (float ms, double sr)
    {
        if (ms <= 0.0f)
            return 1.0f;

        return 1.0f - std::exp (-1.0f / (0.001f * ms * (float) sr));
    }
}

BassendProcessor::BassendProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                          .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    depthParam   = apvts.getRawParameterValue ("depth");
    attackParam  = apvts.getRawParameterValue ("attack");
    holdParam    = apvts.getRawParameterValue ("hold");
    releaseParam = apvts.getRawParameterValue ("release");
}

juce::AudioProcessorValueTreeState::ParameterLayout BassendProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "depth", 1 }, "Depth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "attack", 1 }, "Attack",
        juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f, 0.5f), 5.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "hold", 1 }, "Hold",
        juce::NormalisableRange<float> (0.0f, 500.0f, 0.1f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "release", 1 }, "Release",
        juce::NormalisableRange<float> (5.0f, 1000.0f, 0.1f, 0.5f), 150.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    return layout;
}

bool BassendProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.inputBuses.size() > 1
        && layouts.inputBuses[1] != juce::AudioChannelSet::stereo()
        && layouts.inputBuses[1] != juce::AudioChannelSet::disabled())
        return false;

    return true;
}

void BassendProcessor::prepareToPlay (double newSampleRate, int)
{
    sampleRate = newSampleRate;
    detectorEnv = 0.0f;
    currentDuck = 0.0f;
    holdCounter = 0;
}

void BassendProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto mainBus = getBusBuffer (buffer, true, 0);
    auto scBus   = getBusBuffer (buffer, true, 1);

    const int numSamples   = mainBus.getNumSamples();
    const int mainChannels = mainBus.getNumChannels();
    const int scChannels   = scBus.getNumChannels();

    const float depth       = depthParam->load() * 0.01f;
    const float attackCoef  = coefFromMs (attackParam->load(), sampleRate);
    const float releaseCoef = coefFromMs (releaseParam->load(), sampleRate);
    const int   holdSamples = (int) (holdParam->load() * 0.001 * sampleRate);

    // Detector fijo: ataque muy rápido para captar el transitorio del kick.
    const float detAttackCoef  = coefFromMs (1.0f, sampleRate);
    const float detReleaseCoef = coefFromMs (80.0f, sampleRate);

    float peakReduction = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        // Nivel del sidechain (máximo de canales).
        float sc = 0.0f;
        for (int ch = 0; ch < scChannels; ++ch)
            sc = std::max (sc, std::abs (scBus.getSample (ch, i)));

        detectorEnv += (sc - detectorEnv) * (sc > detectorEnv ? detAttackCoef : detReleaseCoef);

        const float target = depth * juce::jlimit (0.0f, 1.0f, detectorEnv);

        // Ataque hacia la reducción, retención y luego liberación.
        if (target > currentDuck)
        {
            currentDuck += (target - currentDuck) * attackCoef;
            holdCounter = holdSamples;
        }
        else if (holdCounter > 0)
        {
            --holdCounter;
        }
        else
        {
            currentDuck += (target - currentDuck) * releaseCoef;
        }

        const float gain = 1.0f - currentDuck;

        for (int ch = 0; ch < mainChannels; ++ch)
            mainBus.getWritePointer (ch)[i] *= gain;

        peakReduction = std::max (peakReduction, currentDuck);
    }

    reductionMeter.store (peakReduction);
}

juce::AudioProcessorEditor* BassendProcessor::createEditor()
{
    return new BassendEditor (*this);
}

void BassendProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void BassendProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// Punto de entrada que usa el wrapper VST3.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BassendProcessor();
}
