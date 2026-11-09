#include "PluginEditor.h"

namespace BassendColours
{
    const juce::Colour background { 0xff050806 };
    const juce::Colour panel      { 0xff121a15 };
    const juce::Colour track      { 0xff1f2a23 };
    const juce::Colour green      { 0xff3fc46b };
    const juce::Colour gold       { 0xffd4af37 };
    const juce::Colour grey       { 0xff7d857f };
    const juce::Colour white      { 0xfff2f2f2 };
}

namespace
{
    // Color de los ojos según la intensidad: blanco -> amarillo -> naranja -> rojo.
    juce::Colour eyeColourFor (float t)
    {
        const juce::Colour stops[] = { juce::Colour (0xfffff8e7), juce::Colour (0xffffd23f),
                                       juce::Colour (0xffff8a1f), juce::Colour (0xffe8201a) };

        const float scaled = juce::jlimit (0.0f, 1.0f, t) * 3.0f;
        const int i = juce::jmin ((int) scaled, 2);
        return stops[i].interpolatedWith (stops[i + 1], scaled - (float) i);
    }

    // Dibuja el búho dentro de un área. t = intensidad 0..1, reduction = reducción actual 0..1.
    void drawOwl (juce::Graphics& g, juce::Rectangle<float> area, float t, float reduction)
    {
        const auto c = area.getCentre();
        const float s = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f * 0.92f;
        const auto eye = eyeColourFor (t);

        // Percha
        g.setColour (juce::Colour (0xff1a1409));
        g.fillRoundedRectangle (c.x - 0.5f * s, c.y + 0.62f * s, s, 0.12f * s, 0.06f * s);
        g.setColour (BassendColours::gold.withAlpha (0.35f));
        g.drawRoundedRectangle (c.x - 0.5f * s, c.y + 0.62f * s, s, 0.12f * s, 0.06f * s, 1.0f);

        // Cuerpo y cabeza
        juce::ColourGradient bodyGrad (juce::Colour (0xff17422f), c.x, c.y - 0.6f * s,
                                       juce::Colour (0xff030a07), c.x, c.y + 0.7f * s, false);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (c.x - 0.42f * s, c.y - 0.1f * s, 0.84f * s, 0.8f * s);
        g.fillEllipse (c.x - 0.40f * s, c.y - 0.6f * s, 0.80f * s, 0.8f * s);

        // Penachos de las orejas
        juce::Path tufts;
        tufts.startNewSubPath (c.x - 0.36f * s, c.y - 0.42f * s);
        tufts.lineTo (c.x - 0.44f * s, c.y - 0.82f * s);
        tufts.lineTo (c.x - 0.14f * s, c.y - 0.58f * s);
        tufts.closeSubPath();
        tufts.startNewSubPath (c.x + 0.36f * s, c.y - 0.42f * s);
        tufts.lineTo (c.x + 0.44f * s, c.y - 0.82f * s);
        tufts.lineTo (c.x + 0.14f * s, c.y - 0.58f * s);
        tufts.closeSubPath();
        g.fillPath (tufts);

        // Ojos: halo que crece con la reducción, disco facial, iris y pupila
        const float eyeR = 0.13f * s;
        const float eyeY = c.y - 0.3f * s;
        const float eyeX[] = { c.x - 0.17f * s, c.x + 0.17f * s };

        for (float x : eyeX)
        {
            g.setColour (eye.withAlpha (0.12f + 0.3f * reduction));
            g.fillEllipse (x - eyeR * 1.9f, eyeY - eyeR * 1.9f, eyeR * 3.8f, eyeR * 3.8f);

            g.setColour (juce::Colour (0xff04110b));
            g.fillEllipse (x - eyeR * 1.25f, eyeY - eyeR * 1.25f, eyeR * 2.5f, eyeR * 2.5f);

            juce::ColourGradient eg (eye, x, eyeY, eye.darker (0.55f), x + eyeR, eyeY + eyeR, true);
            g.setGradientFill (eg);
            g.fillEllipse (x - eyeR, eyeY - eyeR, eyeR * 2.0f, eyeR * 2.0f);

            g.setColour (juce::Colour (0xff050505));
            g.fillEllipse (x - eyeR * 0.35f, eyeY - eyeR * 0.35f, eyeR * 0.7f, eyeR * 0.7f);
        }

        // Pico
        juce::Path beak;
        beak.startNewSubPath (c.x - 0.05f * s, c.y - 0.06f * s);
        beak.lineTo (c.x + 0.05f * s, c.y - 0.06f * s);
        beak.lineTo (c.x, c.y + 0.04f * s);
        beak.closeSubPath();
        g.setColour (juce::Colour (0xffb8862a));
        g.fillPath (beak);

        // Plumas laterales
        g.setColour (BassendColours::gold.withAlpha (0.22f));
        for (float side : { -1.0f, 1.0f })
        {
            juce::Path wing;
            wing.addCentredArc (c.x + side * 0.26f * s, c.y + 0.3f * s, 0.12f * s, 0.3f * s,
                                0.0f, 0.0f, juce::MathConstants<float>::pi, true);
            g.strokePath (wing, juce::PathStrokeType (1.2f));
        }
    }
}

//==============================================================================
void BassendLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float startAngle, float endAngle,
                                           juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (10.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // Sombra
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (centre.x - radius + 3.0f, centre.y - radius + 6.0f, radius * 2.0f, radius * 2.0f);

    // Anillo exterior oscuro
    g.setColour (juce::Colour (0xff0b0f0c));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    // Escala con marcas: iluminadas hasta la posición actual
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * ((float) i / 10.0f);
        const bool lit = a <= angle + 0.001f;
        g.setColour (lit ? BassendColours::gold : BassendColours::grey.withAlpha (0.5f));

        juce::Line<float> tick (centre.getPointOnCircumference (radius - 4.0f, a),
                                centre.getPointOnCircumference (radius - 11.0f, a));
        g.drawLine (tick, 2.0f);
    }

    // Arco de valor en verde
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, radius - 16.0f, radius - 16.0f, 0.0f, startAngle, angle, true);
    g.setColour (BassendColours::green);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f));

    // Cuerpo metálico con degradado radial
    const float knobR = radius * 0.72f;
    juce::ColourGradient body (juce::Colour (0xff6e746f), centre.x - knobR * 0.4f, centre.y - knobR * 0.5f,
                               juce::Colour (0xff0d100e), centre.x + knobR, centre.y + knobR, true);
    g.setGradientFill (body);
    g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);

    // Estrías del knurling
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (int i = 0; i < 60; ++i)
    {
        const float a = (float) i * juce::MathConstants<float>::twoPi / 60.0f;
        juce::Line<float> rib (centre.getPointOnCircumference (knobR * 0.86f, a),
                               centre.getPointOnCircumference (knobR * 0.98f, a));
        g.drawLine (rib, 1.0f);
    }

    // Borde con brillo
    g.setColour (juce::Colours::white.withAlpha (0.15f));
    g.drawEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f, 1.5f);

    // Puntero dorado
    juce::Line<float> pointer (centre.getPointOnCircumference (knobR * 0.25f, angle),
                               centre.getPointOnCircumference (knobR * 0.9f, angle));
    g.setColour (BassendColours::gold);
    g.drawLine (pointer, 3.0f);
}

//==============================================================================
BassendEditor::BassendEditor (BassendProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (640, 380);

    intensityKnob.setLookAndFeel (&lookAndFeel);
    intensityKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    intensityKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (intensityKnob);

    intensityLabel.setJustificationType (juce::Justification::centred);
    intensityLabel.setColour (juce::Label::textColourId, BassendColours::gold);
    addAndMakeVisible (intensityLabel);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "intensity", intensityKnob);

    // Presets: 25, 50, 75 y 100 %
    const char* presetNames[] = { "SUAVE", "MEDIO", "ALTO", "EXTREMO" };
    const float presetValues[] = { 25.0f, 50.0f, 75.0f, 100.0f };

    for (int i = 0; i < 4; ++i)
    {
        auto& b = presetButtons[i];
        b.setButtonText (presetNames[i]);
        b.setClickingTogglesState (false);
        b.setColour (juce::TextButton::buttonColourId, BassendColours::panel);
        b.setColour (juce::TextButton::buttonOnColourId, BassendColours::gold);
        b.setColour (juce::TextButton::textColourOffId, BassendColours::grey);
        b.setColour (juce::TextButton::textColourOnId, BassendColours::background);
        b.onClick = [this, value = presetValues[i]]
        {
            intensityKnob.setValue (value, juce::sendNotificationSync);
        };
        addAndMakeVisible (b);
    }

    startTimerHz (30);
}

BassendEditor::~BassendEditor()
{
    stopTimer();
    intensityKnob.setLookAndFeel (nullptr);
}

void BassendEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds();

    // Fondo con brillo verde en el centro, como niebla detrás del búho
    juce::ColourGradient bg (juce::Colour (0xff10291c), bounds.getWidth() * 0.7f, bounds.getHeight() * 0.45f,
                             BassendColours::background, bounds.getWidth() * 0.1f, bounds.getHeight() * 1.2f, true);
    g.setGradientFill (bg);
    g.fillAll();

    // Cabecera
    g.setColour (BassendColours::gold);
    g.setFont (juce::FontOptions (18.0f));
    g.drawText ("NARMIA.AI", 22, 0, 200, 50, juce::Justification::centredLeft);
    g.setColour (BassendColours::white);
    g.drawText ("BASSEND", bounds.getWidth() - 222, 0, 200, 50, juce::Justification::centredRight);

    // Búho a la derecha del knob
    const auto owlArea = juce::Rectangle<float> (bounds.getWidth() * 0.55f, 50.0f,
                                                 bounds.getWidth() * 0.45f - 10.0f,
                                                 (float) bounds.getHeight() - 90.0f);
    drawOwl (g, owlArea, displayedIntensity * 0.01f, displayedReduction);

    // Medidor de reducción de ganancia
    auto meter = juce::Rectangle<float> (24.0f, (float) bounds.getHeight() - 28.0f,
                                         (float) bounds.getWidth() - 48.0f, 10.0f);
    g.setColour (BassendColours::panel);
    g.fillRoundedRectangle (meter, 5.0f);

    g.setColour (BassendColours::green);
    g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * juce::jlimit (0.0f, 1.0f, displayedReduction)), 5.0f);
}

void BassendEditor::resized()
{
    auto area = getLocalBounds();
    area.removeFromTop (50);
    area.removeFromBottom (36);

    auto left = area.removeFromLeft (juce::roundToInt (getWidth() * 0.55f)).reduced (20, 8);

    auto buttons = left.removeFromBottom (40);
    auto label   = left.removeFromBottom (26);

    const int knobSize = juce::jmin (juce::jmin (left.getWidth(), left.getHeight()), 260);
    intensityKnob.setBounds (left.withSizeKeepingCentre (knobSize, knobSize));
    intensityLabel.setBounds (label);

    const int buttonWidth = buttons.getWidth() / 4;
    for (auto& b : presetButtons)
        b.setBounds (buttons.removeFromLeft (buttonWidth).reduced (4, 6));
}

void BassendEditor::timerCallback()
{
    const float reduction = processor.reductionMeter.load();
    const float intensity = (float) intensityKnob.getValue();

    bool changed = false;

    if (std::abs (reduction - displayedReduction) > 0.001f)
    {
        displayedReduction = reduction;
        changed = true;
    }

    if (std::abs (intensity - displayedIntensity) > 0.01f)
    {
        displayedIntensity = intensity;
        intensityLabel.setText ("INTENSITY  " + juce::String (juce::roundToInt (intensity)) + " %",
                                juce::dontSendNotification);

        // Resalta el preset cuyo valor coincide con el knob
        const float presetValues[] = { 25.0f, 50.0f, 75.0f, 100.0f };
        for (int i = 0; i < 4; ++i)
            presetButtons[i].setToggleState (std::abs (intensity - presetValues[i]) < 0.5f,
                                             juce::dontSendNotification);
        changed = true;
    }

    if (changed)
        repaint();
}
