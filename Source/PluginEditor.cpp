#include "PluginEditor.h"

SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor(SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&retroLookAndFeel);

    // Preset Selector
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.0f, juce::Font::bold));
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::textMuted);
    addAndMakeVisible(presetLabel);

    for (int i = 0; i < audioProcessor.getNumPrograms(); ++i)
        presetBox.addItem(audioProcessor.getProgramName(i), i + 1);

    presetBox.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]()
    {
        int selected = presetBox.getSelectedId() - 1;
        if (selected >= 0 && selected < audioProcessor.getNumPrograms())
            audioProcessor.setCurrentProgram(selected);
    };
    addAndMakeVisible(presetBox);

    // Setup ComboBoxes
    osc1WaveBox.addItemList({ "Pulse (PWM)", "NES 4-bit Tri", "Sawtooth", "Sine", "LFSR Noise" }, 1);
    setupComboBox(osc1WaveBox, osc1WaveLabel, "Waveform");

    osc2WaveBox.addItemList({ "Pulse (PWM)", "NES 4-bit Tri", "Sawtooth", "Sine", "LFSR Noise" }, 1);
    setupComboBox(osc2WaveBox, osc2WaveLabel, "Waveform");

    arpModeBox.addItemList({ "Off", "Up", "Down", "Up/Down", "Chiptune Maj", "Chiptune Min", "Chiptune Oct" }, 1);
    setupComboBox(arpModeBox, arpModeLabel, "Pattern");

    arpRateBox.addItemList({ "1/8", "1/16", "1/32", "1/64" }, 1);
    setupComboBox(arpRateBox, arpRateLabel, "Division");

    // OSC 1 Sliders
    setupSlider(osc1PwSlider, osc1PwLabel, "Pulse Width", "Osc1Pw");
    osc1PwSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    osc1PwSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    // OSC 2 Enable Toggle
    addAndMakeVisible(osc2EnableToggle);
    osc2EnableToggle.setButtonText("ON");
    osc2EnableToggle.onClick = [this]() { updateOsc2EnabledState(); };

    // OSC 2 Sliders
    setupSlider(osc2OctSlider, osc2OctLabel, "Octave", "Osc2Oct");
    osc2OctSlider.textFromValueFunction = [](double val) {
        int v = juce::roundToInt(val);
        return (v > 0 ? "+" : "") + juce::String(v) + " oct";
    };
    osc2OctSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789").getDoubleValue(); };

    setupSlider(osc2SemiSlider, osc2SemiLabel, "Semitone", "Osc2Semi");
    osc2SemiSlider.textFromValueFunction = [](double val) {
        int v = juce::roundToInt(val);
        return (v > 0 ? "+" : "") + juce::String(v) + " st";
    };
    osc2SemiSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789").getDoubleValue(); };

    setupSlider(osc2DetuneSlider, osc2DetuneLabel, "Fine Tune", "Osc2Detune");
    osc2DetuneSlider.textFromValueFunction = [](double val) {
        return (val > 0.0 ? "+" : "") + juce::String(val, 1) + " ct";
    };
    osc2DetuneSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789.").getDoubleValue(); };

    setupSlider(oscMixSlider, oscMixLabel, "Level", "OscMix");
    oscMixSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    oscMixSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    // Envelope Sliders
    setupSlider(attackSlider, attackLabel, "Attack", "EnvAttack");
    attackSlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    attackSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    setupSlider(decaySlider, decayLabel, "Decay", "EnvDecay");
    decaySlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    decaySlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    setupSlider(sustainSlider, sustainLabel, "Sustain", "EnvSustain");
    sustainSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    sustainSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    setupSlider(releaseSlider, releaseLabel, "Release", "EnvRelease");
    releaseSlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    releaseSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    // Filter Sliders
    setupSlider(cutoffSlider, cutoffLabel, "Cutoff", "FilterCutoff");
    cutoffSlider.textFromValueFunction = [](double val) {
        if (val >= 1000.0) return juce::String(val * 0.001, 1) + " kHz";
        return juce::String(juce::roundToInt(val)) + " Hz";
    };
    cutoffSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("k")) return text.retainCharacters("0123456789.").getDoubleValue() * 1000.0;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    setupSlider(resSlider, resLabel, "Resonance", "FilterRes");
    resSlider.textFromValueFunction = [](double val) {
        int pct = juce::roundToInt((val - 0.1) / (5.0 - 0.1) * 100.0);
        return juce::String(pct) + " %";
    };
    resSlider.valueFromTextFunction = [](const juce::String& text) {
        double pct = text.retainCharacters("0123456789.").getDoubleValue() * 0.01;
        return 0.1 + pct * (5.0 - 0.1);
    };

    // Lo-Fi Crunch Sliders
    setupSlider(bitDepthSlider, bitDepthLabel, "Bits", "BitDepth");
    bitDepthSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val)) + " bit"; };
    bitDepthSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("0123456789.").getDoubleValue(); };

    setupSlider(downsampleSlider, downsampleLabel, "Crunch", "Downsample");
    downsampleSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val)) + "x"; };
    downsampleSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("0123456789.").getDoubleValue(); };

    // Master Slider
    setupSlider(masterGainSlider, masterGainLabel, "Volume", "MasterGain");
    masterGainSlider.textFromValueFunction = [](double val) { return juce::String(val, 1) + " dB"; };
    masterGainSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789.").getDoubleValue(); };

    // Update text formatting on all sliders
    osc1PwSlider.updateText();
    osc2OctSlider.updateText();
    osc2SemiSlider.updateText();
    osc2DetuneSlider.updateText();
    oscMixSlider.updateText();
    attackSlider.updateText();
    decaySlider.updateText();
    sustainSlider.updateText();
    releaseSlider.updateText();
    cutoffSlider.updateText();
    resSlider.updateText();
    bitDepthSlider.updateText();
    downsampleSlider.updateText();
    masterGainSlider.updateText();

    // Attachments
    auto& apvts = audioProcessor.getAPVTS();

    osc1WaveAttach = std::make_unique<ComboAttachment>(apvts, "osc1_wave", osc1WaveBox);
    osc1PwAttach   = std::make_unique<SliderAttachment>(apvts, "osc1_pw", osc1PwSlider);

    osc2EnableAttach = std::make_unique<ButtonAttachment>(apvts, "osc2_enabled", osc2EnableToggle);
    osc2WaveAttach   = std::make_unique<ComboAttachment>(apvts, "osc2_wave", osc2WaveBox);
    osc2OctAttach    = std::make_unique<SliderAttachment>(apvts, "osc2_octave", osc2OctSlider);
    osc2SemiAttach   = std::make_unique<SliderAttachment>(apvts, "osc2_semi", osc2SemiSlider);
    osc2DetuneAttach = std::make_unique<SliderAttachment>(apvts, "osc2_detune", osc2DetuneSlider);
    oscMixAttach     = std::make_unique<SliderAttachment>(apvts, "osc_mix", oscMixSlider);

    updateOsc2EnabledState();

    attackAttach  = std::make_unique<SliderAttachment>(apvts, "env_attack", attackSlider);
    decayAttach   = std::make_unique<SliderAttachment>(apvts, "env_decay", decaySlider);
    sustainAttach = std::make_unique<SliderAttachment>(apvts, "env_sustain", sustainSlider);
    releaseAttach = std::make_unique<SliderAttachment>(apvts, "env_release", releaseSlider);

    cutoffAttach = std::make_unique<SliderAttachment>(apvts, "filter_cutoff", cutoffSlider);
    resAttach    = std::make_unique<SliderAttachment>(apvts, "filter_res", resSlider);

    bitDepthAttach   = std::make_unique<SliderAttachment>(apvts, "bit_depth", bitDepthSlider);
    downsampleAttach = std::make_unique<SliderAttachment>(apvts, "downsample", downsampleSlider);

    arpModeAttach = std::make_unique<ComboAttachment>(apvts, "arp_mode", arpModeBox);
    arpRateAttach = std::make_unique<ComboAttachment>(apvts, "arp_rate", arpRateBox);

    masterGainAttach = std::make_unique<SliderAttachment>(apvts, "master_gain", masterGainSlider);

    setSize(860, 540);
    startTimerHz(30);
}

SimpleSynthAudioProcessorEditor::~SimpleSynthAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void SimpleSynthAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label,
                                                  const juce::String& text, const juce::String& name)
{
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 16);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::textMuted);
    addAndMakeVisible(label);
}

void SimpleSynthAudioProcessorEditor::setupComboBox(juce::ComboBox& box, juce::Label& label,
                                                    const juce::String& text)
{
    addAndMakeVisible(box);

    label.setText(text, juce::dontSendNotification);
    label.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centredLeft);
    label.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::textMuted);
    addAndMakeVisible(label);
}

void SimpleSynthAudioProcessorEditor::updateOsc2EnabledState()
{
    bool enabled = osc2EnableToggle.getToggleState();
    osc2WaveBox.setEnabled(enabled);
    osc2OctSlider.setEnabled(enabled);
    osc2SemiSlider.setEnabled(enabled);
    osc2DetuneSlider.setEnabled(enabled);
    oscMixSlider.setEnabled(enabled);
    repaint();
}

void SimpleSynthAudioProcessorEditor::timerCallback()
{
    // Retrieve latest scope buffer from processor
    audioProcessor.getVisualizerData(visPoints.data(), numVisPoints);

    if (!visualizerArea.isEmpty())
        repaint(visualizerArea.expanded(4));

    // Repaint Arpeggiator step LED row for live animation
    repaint(530, 160, 316, 75);

    // Keep preset selector synchronized
    int currentProg = audioProcessor.getCurrentProgram();
    if (presetBox.getSelectedId() != currentProg + 1)
        presetBox.setSelectedId(currentProg + 1, juce::dontSendNotification);

    bool osc2Enabled = osc2EnableToggle.getToggleState();
    if (osc2WaveBox.isEnabled() != osc2Enabled)
        updateOsc2EnabledState();
}

void SimpleSynthAudioProcessorEditor::drawSectionPanel(juce::Graphics& g, const juce::Rectangle<int>& area,
                                                       const juce::String& title, juce::Colour /*accentColour*/)
{
    auto bounds = area.toFloat();

    // Solid hardware plate with crisp 3px corner radius
    g.setColour(RetroUI::RetroLookAndFeel::panelBg);
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Section title in Authentic Pixel Font
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(13.0f));
    g.drawText(title, area.getX() + 14, area.getY() + 8, area.getWidth() - 28, 18, juce::Justification::centredLeft);
}

void SimpleSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 1. Global chassis background (#15161A)
    g.fillAll(RetroUI::RetroLookAndFeel::bgChassis);

    // 2. Top Header Banner
    g.setColour(RetroUI::RetroLookAndFeel::panelBg.darker(0.12f));
    g.fillRect(0, 0, getWidth(), 46);

    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawHorizontalLine(46, 0.0f, static_cast<float>(getWidth()));

    // Title in CairoPixel Font
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(16.0f));
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.drawText("RETRO-BIT SYNTH", 18, 6, 260, 22, juce::Justification::centredLeft);

    // Hardware subtitle
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.0f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("8-BIT / 16-BIT HARDWARE HYBRID", 18, 26, 320, 16, juce::Justification::centredLeft);

    // ==========================================
    // ROW 1: SOUND GENERATION (y = 54, h = 220)
    // ==========================================

    // Panel 1: OSCILLATORS (x = 14, y = 54, w = 506, h = 220)
    drawSectionPanel(g, { 14, 54, 506, 220 }, "OSCILLATORS", RetroUI::RetroLookAndFeel::textBright);

    // Sub-section headings inside OSCILLATORS
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(11.5f));
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.drawText("OSC 1", 28, 77, 80, 18, juce::Justification::centredLeft);
    g.drawText("OSC 2", 224, 77, 50, 18, juce::Justification::centredLeft);

    // Hairline divider between OSC 1 and OSC 2
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawVerticalLine(210, 76.0f, 264.0f);

    // Dim OSC 2 area visually when disabled
    if (!osc2EnableToggle.getToggleState())
    {
        auto osc2Area = juce::Rectangle<float>(212.0f, 74.0f, 302.0f, 194.0f);
        g.setColour(RetroUI::RetroLookAndFeel::bgChassis.withAlpha(0.55f));
        g.fillRoundedRectangle(osc2Area, 3.0f);
    }

    // Panel 2: ARPEGGIATOR (x = 530, y = 54, w = 316, h = 220)
    drawSectionPanel(g, { 530, 54, 316, 220 }, "ARPEGGIATOR", RetroUI::RetroLookAndFeel::textBright);

    // Step Active label
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::bold));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("STEP ACTIVE", 546, 162, 120, 14, juce::Justification::centredLeft);

    // 8 Hardware-style Arp Step LEDs in recessed well
    auto stepWell = juce::Rectangle<float>(546.0f, 180.0f, 284.0f, 26.0f);
    g.setColour(RetroUI::RetroLookAndFeel::wellBg);
    g.fillRoundedRectangle(stepWell, 3.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(stepWell, 3.0f, 1.0f);

    int curStep = audioProcessor.getArpStep();
    bool isArpRunning = audioProcessor.isArpActive();
    int arpModeIdx = arpModeBox.getSelectedId() - 1;

    int patternLen = 8;
    if (arpModeIdx == 0) patternLen = 0;
    else if (arpModeIdx >= 4) patternLen = 4; // Chiptune chords (Maj, Min, Oct)

    const float ledW = 27.0f;
    const float ledH = 18.0f;
    const float ledY = stepWell.getY() + 4.0f;
    const float gap = 7.0f;
    const float startX = stepWell.getX() + 6.0f;

    for (int i = 0; i < 8; ++i)
    {
        float lx = startX + i * (ledW + gap);
        juce::Rectangle<float> ledRect(lx, ledY, ledW, ledH);

        bool isCurrent = isArpRunning && (curStep % (patternLen > 0 ? patternLen : 8) == i);
        bool inPattern = (i < patternLen);

        if (isCurrent)
        {
            // Bright illuminated active LED with bloom
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce.withAlpha(0.35f));
            g.fillRoundedRectangle(ledRect.expanded(2.0f), 2.0f);

            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce);
            g.drawRoundedRectangle(ledRect, 2.0f, 1.5f);
        }
        else if (inPattern)
        {
            // Active step in sequence pattern
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce.withAlpha(0.20f));
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce.withAlpha(0.60f));
            g.drawRoundedRectangle(ledRect, 2.0f, 1.0f);
        }
        else
        {
            // Inactive / idle step
            g.setColour(RetroUI::RetroLookAndFeel::panelBg.darker(0.3f));
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
            g.drawRoundedRectangle(ledRect, 2.0f, 1.0f);
        }

        // Step number 1..8
        g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.0f, juce::Font::bold));
        g.setColour(isCurrent ? juce::Colours::black : (inPattern ? RetroUI::RetroLookAndFeel::textBright : RetroUI::RetroLookAndFeel::textDim));
        g.drawText(juce::String(i + 1), ledRect.toNearestInt(), juce::Justification::centred);
    }

    // Arpeggiator Status Readout
    juce::String arpInfo;
    if (arpModeIdx == 0)
    {
        arpInfo = "MODE: OFF (POLYPHONIC 8-VOICE)";
    }
    else
    {
        const char* rates[] = { "1/8 NOTE", "1/16 NOTE", "1/32 NOTE", "1/64 NOTE" };
        int rIdx = juce::jlimit(0, 3, arpRateBox.getSelectedId() - 1);
        arpInfo = juce::String("RATE: ") + rates[rIdx] + "  |  STEPS: " + juce::String(patternLen) + "  |  SYNC: HOST BPM";
    }
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText(arpInfo, stepWell.withY(stepWell.getBottom() + 4.0f).withHeight(16.0f).toNearestInt(), juce::Justification::centred);

    // ==========================================
    // ROW 2: CONTINUOUS LOWER PLATE (y = 284, h = 244)
    // ==========================================
    auto lowerBounds = juce::Rectangle<float>(14.0f, 284.0f, 832.0f, 244.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBg);
    g.fillRoundedRectangle(lowerBounds, 3.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(lowerBounds, 3.0f, 1.0f);

    // Hairline vertical dividers separating Envelope, Filter/Crunch, and Master
    g.drawVerticalLine(288, lowerBounds.getY() + 10.0f, lowerBounds.getBottom() - 10.0f);
    g.drawVerticalLine(566, lowerBounds.getY() + 10.0f, lowerBounds.getBottom() - 10.0f);

    // Section Titles in CairoPixel Font
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(13.0f));
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.drawText("ENVELOPE (ADSR)", 28, 296, 200, 18, juce::Justification::centredLeft);
    g.drawText("FILTER / CRUNCH", 302, 296, 200, 18, juce::Justification::centredLeft);
    g.drawText("MASTER", 580, 296, 120, 18, juce::Justification::centredLeft);

    // Paint Integrated Hardware CRT Oscilloscope
    if (!visualizerArea.isEmpty())
    {
        auto vBounds = visualizerArea.toFloat();

        // 1. Recessed dark screen housing
        g.setColour(juce::Colour::fromRGB(8, 12, 10));
        g.fillRoundedRectangle(vBounds, 3.0f);

        // 2. Bezel border
        g.setColour(RetroUI::RetroLookAndFeel::panelBorder.darker(0.2f));
        g.drawRoundedRectangle(vBounds, 3.0f, 1.0f);

        // 3. Phosphor Grid Reticle
        g.setColour(juce::Colour::fromRGB(18, 28, 22));
        float midY = vBounds.getCentreY();
        float midX = vBounds.getCentreX();
        g.drawLine(vBounds.getX(), midY, vBounds.getRight(), midY, 1.0f);
        g.drawLine(midX, vBounds.getY(), midX, vBounds.getBottom(), 1.0f);

        // Subdivisions
        g.drawLine(vBounds.getX(), midY - vBounds.getHeight() * 0.25f, vBounds.getRight(), midY - vBounds.getHeight() * 0.25f, 0.6f);
        g.drawLine(vBounds.getX(), midY + vBounds.getHeight() * 0.25f, vBounds.getRight(), midY + vBounds.getHeight() * 0.25f, 0.6f);

        // 4. Trace path calculation
        juce::Path wavePath;
        float halfH = vBounds.getHeight() * 0.38f;
        float dx = vBounds.getWidth() / static_cast<float>(numVisPoints);

        for (int i = 0; i < numVisPoints; ++i)
        {
            float x = vBounds.getX() + i * dx;
            float y = midY - juce::jlimit(-1.0f, 1.0f, visPoints[static_cast<size_t>(i)]) * halfH;

            if (i == 0)
                wavePath.startNewSubPath(x, y);
            else
                wavePath.lineTo(x, y);
        }

        // 5. Phosphor Bloom Glow (Pass 1)
        g.setColour(juce::Colour::fromRGB(0, 240, 180).withAlpha(0.20f));
        g.strokePath(wavePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 6. Crisp Beam Core (Pass 2)
        g.setColour(juce::Colour::fromRGB(110, 255, 215).withAlpha(0.95f));
        g.strokePath(wavePath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 7. CRT Horizontal Scanlines
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        for (float yLine = vBounds.getY(); yLine < vBounds.getBottom(); yLine += 3.0f)
        {
            g.drawHorizontalLine(static_cast<int>(yLine), vBounds.getX(), vBounds.getRight());
        }

        // 8. Glass tube vignette
        juce::ColourGradient vignette(juce::Colours::transparentBlack, midX, midY,
                                      juce::Colour::fromRGB(2, 4, 3).withAlpha(0.40f), vBounds.getX(), vBounds.getY(), true);
        g.setGradientFill(vignette);
        g.fillRoundedRectangle(vBounds, 3.0f);
    }
}

void SimpleSynthAudioProcessorEditor::resized()
{
    // Preset dropdown in header
    presetLabel.setBounds(540, 11, 60, 24);
    presetBox.setBounds(605, 11, 235, 24);

    // ==========================================
    // ROW 1: SOUND GENERATION (y = 54, h = 220)
    // ==========================================

    // OSC 1 (Left quadrant)
    osc1WaveLabel.setBounds(28, 100, 164, 14);
    osc1WaveBox.setBounds(28, 118, 164, 24);
    osc1PwLabel.setBounds(75, 154, 70, 14);
    osc1PwSlider.setBounds(75, 170, 70, 88);

    // OSC 2 (Right quadrant)
    osc2EnableToggle.setBounds(276, 76, 56, 20);
    osc2WaveLabel.setBounds(224, 100, 280, 14);
    osc2WaveBox.setBounds(224, 118, 280, 24);

    const int osc2KnobY = 170;
    const int osc2KnobW = 64;
    const int osc2KnobH = 88;
    const int osc2LblY  = 154;

    osc2OctLabel.setBounds(224, osc2LblY, osc2KnobW, 14);
    osc2OctSlider.setBounds(224, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2SemiLabel.setBounds(296, osc2LblY, osc2KnobW, 14);
    osc2SemiSlider.setBounds(296, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2DetuneLabel.setBounds(368, osc2LblY, osc2KnobW, 14);
    osc2DetuneSlider.setBounds(368, osc2KnobY, osc2KnobW, osc2KnobH);

    oscMixLabel.setBounds(440, osc2LblY, osc2KnobW, 14);
    oscMixSlider.setBounds(440, osc2KnobY, osc2KnobW, osc2KnobH);

    // ARPEGGIATOR (x = 530, y = 54, w = 316, h = 220)
    arpModeLabel.setBounds(546, 98, 66, 22);
    arpModeBox.setBounds(618, 96, 212, 24);

    arpRateLabel.setBounds(546, 130, 66, 22);
    arpRateBox.setBounds(618, 128, 212, 24);

    // ==========================================
    // ROW 2: CONTINUOUS LOWER PLATE (y = 284, h = 244)
    // ==========================================

    // ENVELOPE (ADSR) (x = 14 to 288)
    const int envKnobW = 58;
    const int envKnobH = 92;
    const int envKnobY = 350;
    const int envLblY  = 334;

    attackLabel.setBounds(26, envLblY, envKnobW, 14);
    attackSlider.setBounds(26, envKnobY, envKnobW, envKnobH);

    decayLabel.setBounds(90, envLblY, envKnobW, 14);
    decaySlider.setBounds(90, envKnobY, envKnobW, envKnobH);

    sustainLabel.setBounds(154, envLblY, envKnobW, 14);
    sustainSlider.setBounds(154, envKnobY, envKnobW, envKnobH);

    releaseLabel.setBounds(218, envLblY, envKnobW, 14);
    releaseSlider.setBounds(218, envKnobY, envKnobW, envKnobH);

    // FILTER / CRUNCH (x = 288 to 566) - 2x2 grid
    const int filterKnobW = 96;
    const int filterKnobH = 76;

    cutoffLabel.setBounds(318, 324, filterKnobW, 14);
    cutoffSlider.setBounds(318, 340, filterKnobW, filterKnobH);

    resLabel.setBounds(438, 324, filterKnobW, 14);
    resSlider.setBounds(438, 340, filterKnobW, filterKnobH);

    bitDepthLabel.setBounds(318, 424, filterKnobW, 14);
    bitDepthSlider.setBounds(318, 440, filterKnobW, filterKnobH);

    downsampleLabel.setBounds(438, 424, filterKnobW, 14);
    downsampleSlider.setBounds(438, 440, filterKnobW, filterKnobH);

    // MASTER & SCOPE (x = 566 to 846)
    masterGainLabel.setBounds(668, 318, 76, 14);
    masterGainSlider.setBounds(668, 334, 76, 76);

    // Recessed widescreen CRT oscilloscope integrated beneath Volume knob
    visualizerArea.setBounds(586, 418, 240, 98);
}
