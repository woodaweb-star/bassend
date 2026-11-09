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
    intensityParam = apvts.getRawParameterValue ("intensity");
}

juce::AudioProcessorValueTreeState::ParameterLayout BassendProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "intensity", 1 }, "Intensity",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 60.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

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
    sampleRate  = newSampleRate;
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

    // Un solo knob (0..1). Cuanto más alto, más profundo, rápido y exagerado el ducking.
    const float n = juce::jlimit (0.0f, 1.0f, intensityParam->load() * 0.01f);

    // Profundidad: crece de forma no lineal para que el tramo alto se note más.
    // Tope en 97 % para no llegar a silencio total.
    const float maxDepth = 0.97f * std::pow (n, 1.2f);

    // Ataque: más rápido al subir la intensidad.
    const float attackCoef = coefFromMs (8.0f - 6.0f * n, sampleRate);

    // Liberación: más corta al subir la intensidad, para un efecto de bombeo más marcado.
    const float releaseCoef = coefFromMs (juce::jmax (60.0f, 260.0f - 200.0f * n), sampleRate);

    // Retención: 20 ms a 300 ms según la intensidad.
    const int holdSamples = (int) ((20.0f + 280.0f * n) * 0.001 * sampleRate);

    // Detector fijo: ataque muy rápido para captar el transitorio del kick.
    const float detAttackCoef  = coefFromMs (1.0f, sampleRate);
    const float detReleaseCoef = coefFromMs (80.0f, sampleRate);

    float peakReduction = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float sc = 0.0f;
        for (int ch = 0; ch < scChannels; ++ch)
            sc = std::max (sc, std::abs (scBus.getSample (ch, i)));

        detectorEnv += (sc - detectorEnv) * (sc > detectorEnv ? detAttackCoef : detReleaseCoef);

        const float target = maxDepth * juce::jlimit (0.0f, 1.0f, detectorEnv);

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
