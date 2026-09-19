#include "PluginEditor.h"

SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor(SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&retroLookAndFeel);

    // Preset Selector
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.5f, juce::Font::bold));
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
    arpModeBox.onChange = [this]() { updateArpEnabledState(); };

    arpRateBox.addItemList({ "1/8", "1/16", "1/32", "1/64" }, 1);
    setupComboBox(arpRateBox, arpRateLabel, "Division");

    // OSC 1 Sliders
    setupSlider(osc1PwSlider, osc1PwLabel, "Pulse Width", "Osc1Pw", 0.50);

    // OSC 2 Enable Toggle
    addAndMakeVisible(osc2EnableToggle);
    osc2EnableToggle.setButtonText("ON");
    osc2EnableToggle.onClick = [this]() { updateOsc2EnabledState(); };

    // OSC 2 Sliders
    setupSlider(osc2OctSlider, osc2OctLabel, "Octave", "Osc2Oct", 0.0);
    setupSlider(osc2SemiSlider, osc2SemiLabel, "Semitone", "Osc2Semi", 0.0);
    setupSlider(osc2DetuneSlider, osc2DetuneLabel, "Fine Tune", "Osc2Detune", 0.0);
    setupSlider(oscMixSlider, oscMixLabel, "Level", "OscMix", 0.0);

    // Envelope Sliders
    setupSlider(attackSlider, attackLabel, "Attack", "EnvAttack", 0.010);
    setupSlider(decaySlider, decayLabel, "Decay", "EnvDecay", 0.150);
    setupSlider(sustainSlider, sustainLabel, "Sustain", "EnvSustain", 0.70);
    setupSlider(releaseSlider, releaseLabel, "Release", "EnvRelease", 0.200);

    // Filter Sliders
    setupSlider(cutoffSlider, cutoffLabel, "Cutoff", "FilterCutoff", 20000.0);
    setupSlider(resSlider, resLabel, "Resonance", "FilterRes", 0.70);

    // Lo-Fi Crunch Sliders
    setupSlider(bitDepthSlider, bitDepthLabel, "Bits", "BitDepth", 16.0);
    setupSlider(downsampleSlider, downsampleLabel, "Crunch", "Downsample", 1.0);

    // Master Slider
    setupSlider(masterGainSlider, masterGainLabel, "Volume", "MasterGain", 0.0);

    // Attachments (constructed first so formatters override attachment defaults)
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

    updateArpEnabledState();

    masterGainAttach = std::make_unique<SliderAttachment>(apvts, "master_gain", masterGainSlider);

    // Precise Human-Scale Value Formatters (Applied post-attachment)
    osc1PwSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    osc1PwSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    osc2OctSlider.textFromValueFunction = [](double val) {
        int v = juce::roundToInt(val);
        return (v > 0 ? "+" : "") + juce::String(v) + " oct";
    };
    osc2OctSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789").getDoubleValue(); };

    osc2SemiSlider.textFromValueFunction = [](double val) {
        int v = juce::roundToInt(val);
        return (v > 0 ? "+" : "") + juce::String(v) + " st";
    };
    osc2SemiSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789").getDoubleValue(); };

    osc2DetuneSlider.textFromValueFunction = [](double val) {
        int c = juce::roundToInt(val);
        return (c > 0 ? "+" : "") + juce::String(c) + " ct";
    };
    osc2DetuneSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("-+0123456789.").getDoubleValue(); };

    oscMixSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    oscMixSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    attackSlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    attackSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    decaySlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    decaySlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    sustainSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val * 100.0)) + " %"; };
    sustainSlider.valueFromTextFunction = [](const juce::String& text) { return text.upToFirstOccurrenceOf("%", false, false).trim().getDoubleValue() * 0.01; };

    releaseSlider.textFromValueFunction = [](double val) {
        if (val < 1.0) return juce::String(juce::roundToInt(val * 1000.0)) + " ms";
        return juce::String(val, 2) + " s";
    };
    releaseSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("ms")) return text.retainCharacters("0123456789.").getDoubleValue() * 0.001;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    cutoffSlider.textFromValueFunction = [](double val) {
        if (val >= 1000.0) return juce::String(val * 0.001, 1) + " kHz";
        return juce::String(juce::roundToInt(val)) + " Hz";
    };
    cutoffSlider.valueFromTextFunction = [](const juce::String& text) {
        if (text.containsIgnoreCase("k")) return text.retainCharacters("0123456789.").getDoubleValue() * 1000.0;
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    resSlider.textFromValueFunction = [](double val) {
        return juce::String(val, 2);
    };
    resSlider.valueFromTextFunction = [](const juce::String& text) {
        return text.retainCharacters("0123456789.").getDoubleValue();
    };

    bitDepthSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val)) + " bit"; };
    bitDepthSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("0123456789.").getDoubleValue(); };

    downsampleSlider.textFromValueFunction = [](double val) { return juce::String(juce::roundToInt(val)) + "x"; };
    downsampleSlider.valueFromTextFunction = [](const juce::String& text) { return text.retainCharacters("0123456789.").getDoubleValue(); };

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

    setSize(860, 540);
    startTimerHz(30);
}

SimpleSynthAudioProcessorEditor::~SimpleSynthAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void SimpleSynthAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label,
                                                  const juce::String& text, const juce::String& name,
                                                  double defaultValue)
{
    slider.setName(name);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 16);
    slider.setDoubleClickReturnValue(true, defaultValue);
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::textMuted);
    addAndMakeVisible(label);
}

void SimpleSynthAudioProcessorEditor::setupComboBox(juce::ComboBox& box, juce::Label& label,
                                                    const juce::String& text)
{
    addAndMakeVisible(box);

    label.setText(text, juce::dontSendNotification);
    label.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.0f, juce::Font::bold));
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
    repaint(14, 54, 506, 220);
}

void SimpleSynthAudioProcessorEditor::updateArpEnabledState()
{
    bool arpEnabled = (arpModeBox.getSelectedId() > 1); // 1 is 'Off'
    arpRateBox.setEnabled(arpEnabled);
    arpRateBox.setAlpha(arpEnabled ? 1.0f : 0.35f);
    arpRateLabel.setAlpha(arpEnabled ? 1.0f : 0.35f);
    repaint(530, 54, 316, 220);
}

void SimpleSynthAudioProcessorEditor::timerCallback()
{
    // Retrieve latest scope buffer from processor
    audioProcessor.getVisualizerData(visPoints.data(), numVisPoints);

    // Track real-time master peak level with smooth decay
    float targetPeak = audioProcessor.getMasterPeakLevel();
    currentMeterLevel = juce::jmax(targetPeak, currentMeterLevel * 0.82f);

    // Repaint master area (includes scope and real-time LED meter)
    if (!visualizerArea.isEmpty())
        repaint(570, 315, 274, 210);

    // Repaint Arpeggiator step LED row for live animation
    repaint(540, 154, 296, 80);

    // Keep preset selector synchronized
    int currentProg = audioProcessor.getCurrentProgram();
    if (presetBox.getSelectedId() != currentProg + 1)
        presetBox.setSelectedId(currentProg + 1, juce::dontSendNotification);

    bool osc2Enabled = osc2EnableToggle.getToggleState();
    if (osc2WaveBox.isEnabled() != osc2Enabled)
        updateOsc2EnabledState();

    bool arpOn = (arpModeBox.getSelectedId() > 1);
    if (arpRateBox.isEnabled() != arpOn)
        updateArpEnabledState();
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

    // Section title in Authentic Pixel Font (identity)
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
    g.drawText("RETRO-BIT SYNTH", 18, 6, 220, 22, juce::Justification::centredLeft);

    // Subtitle in clean sans-serif
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.0f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("8-BIT / 16-BIT HARDWARE HYBRID", 18, 26, 260, 16, juce::Justification::centredLeft);

    // Quiet voice badge in header near Preset selector
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.5f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted.withAlpha(0.85f));
    g.drawText("8-Voice Poly", 440, 15, 84, 16, juce::Justification::centredRight);

    // ==========================================
    // ROW 1: SOUND GENERATION (y = 54, h = 220)
    // ==========================================

    // Panel 1: OSCILLATORS (x = 14, y = 54, w = 506, h = 220)
    drawSectionPanel(g, { 14, 54, 506, 220 }, "OSCILLATORS", RetroUI::RetroLookAndFeel::textBright);

    // Sub-section headings inside OSCILLATORS (symmetrical visual weight)
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(12.0f));
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.drawText("OSC 1", 28, 74, 80, 18, juce::Justification::centredLeft);
    g.drawText("OSC 2", 270, 74, 54, 18, juce::Justification::centredLeft);

    // Toned-down hairline vertical divider between OSC 1 and OSC 2
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder.withAlpha(0.35f));
    g.drawVerticalLine(256, 74.0f, 264.0f);

    // Subtle tick between tuning group and level in OSC 2
    g.drawVerticalLine(451, 148.0f, 252.0f);

    // Aggressive dimming for disabled OSC 2
    if (!osc2EnableToggle.getToggleState())
    {
        auto osc2Area = juce::Rectangle<float>(258.0f, 72.0f, 260.0f, 198.0f);
        g.setColour(RetroUI::RetroLookAndFeel::bgChassis.withAlpha(0.55f));
        g.fillRoundedRectangle(osc2Area, 3.0f);
    }

    // Panel 2: ARPEGGIATOR (x = 530, y = 54, w = 316, h = 220)
    drawSectionPanel(g, { 530, 54, 316, 220 }, "ARPEGGIATOR", RetroUI::RetroLookAndFeel::textBright);

    // Steps label in clean sans-serif
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.0f, juce::Font::bold));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("Steps", 546, 156, 100, 14, juce::Justification::centredLeft);

    // 8 Hardware-style Arp Step LEDs in recessed well
    auto stepWell = juce::Rectangle<float>(546.0f, 174.0f, 284.0f, 28.0f);
    g.setColour(RetroUI::RetroLookAndFeel::wellBg);
    g.fillRoundedRectangle(stepWell, 3.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(stepWell, 3.0f, 1.0f);

    int curStep = audioProcessor.getArpStep();
    bool isArpRunning = audioProcessor.isArpActive();
    int arpModeIdx = arpModeBox.getSelectedId() - 1;
    bool arpEnabled = (arpModeIdx > 0);

    int patternLen = 8;
    if (!arpEnabled) patternLen = 0;
    else if (arpModeIdx >= 4) patternLen = 4; // Chiptune chords (Maj, Min, Oct)

    const float ledW = 27.0f;
    const float ledH = 20.0f;
    const float ledY = stepWell.getY() + 4.0f;
    const float gap = 7.0f;
    const float startX = stepWell.getX() + 6.0f;

    for (int i = 0; i < 8; ++i)
    {
        float lx = startX + i * (ledW + gap);
        juce::Rectangle<float> ledRect(lx, ledY, ledW, ledH);

        bool isCurrent = arpEnabled && isArpRunning && (curStep % (patternLen > 0 ? patternLen : 8) == i);
        bool inPattern = arpEnabled && (i < patternLen);

        if (isCurrent)
        {
            // State 3: Current / Playhead Step (luminous bloom + bright solid core)
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce.withAlpha(0.40f));
            g.fillRoundedRectangle(ledRect.expanded(2.0f), 2.0f);

            g.setColour(RetroUI::RetroLookAndFeel::cyanIce);
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(juce::Colours::white);
            g.drawRoundedRectangle(ledRect, 2.0f, 1.5f);

            g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::bold));
            g.setColour(juce::Colour::fromRGB(10, 14, 20));
            g.drawText(juce::String(i + 1), ledRect.toNearestInt(), juce::Justification::centred);
        }
        else if (inPattern)
        {
            // State 2: Active Step in pattern (filled soft cyan + cyan border)
            g.setColour(juce::Colour::fromRGB(19, 52, 61));
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(RetroUI::RetroLookAndFeel::cyanIce.withAlpha(0.70f));
            g.drawRoundedRectangle(ledRect, 2.0f, 1.0f);

            g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::bold));
            g.setColour(RetroUI::RetroLookAndFeel::textBright);
            g.drawText(juce::String(i + 1), ledRect.toNearestInt(), juce::Justification::centred);
        }
        else
        {
            // State 1: Inactive Step (dark matte gray)
            g.setColour(juce::Colour::fromRGB(21, 23, 28));
            g.fillRoundedRectangle(ledRect, 2.0f);
            g.setColour(RetroUI::RetroLookAndFeel::panelBorder.darker(0.1f));
            g.drawRoundedRectangle(ledRect, 2.0f, 1.0f);

            g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::plain));
            g.setColour(RetroUI::RetroLookAndFeel::textDim);
            g.drawText(juce::String(i + 1), ledRect.toNearestInt(), juce::Justification::centred);
        }
    }

    // Arpeggiator Status text in clean sans-serif
    juce::String arpInfo;
    if (!arpEnabled)
    {
        arpInfo = "Arpeggiator Disabled";
    }
    else
    {
        const char* rates[] = { "1/8 Note", "1/16 Note", "1/32 Note", "1/64 Note" };
        int rIdx = juce::jlimit(0, 3, arpRateBox.getSelectedId() - 1);
        arpInfo = juce::String(rates[rIdx]) + " · " + juce::String(patternLen) + " Steps · Host Synced";
    }
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::plain));
    g.setColour(arpEnabled ? RetroUI::RetroLookAndFeel::textMuted : RetroUI::RetroLookAndFeel::textDim);
    g.drawText(arpInfo, stepWell.withY(stepWell.getBottom() + 4.0f).withHeight(16.0f).toNearestInt(), juce::Justification::centred);

    // Aggressive dimming over Arpeggiator body when Pattern = Off
    if (!arpEnabled)
    {
        auto arpScrim = juce::Rectangle<float>(532.0f, 120.0f, 312.0f, 150.0f);
        g.setColour(RetroUI::RetroLookAndFeel::bgChassis.withAlpha(0.55f));
        g.fillRoundedRectangle(arpScrim, 3.0f);
    }

    // ==========================================
    // ROW 2: CONTINUOUS LOWER PLATE (y = 284, h = 244)
    // ==========================================
    auto lowerBounds = juce::Rectangle<float>(14.0f, 284.0f, 832.0f, 244.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBg);
    g.fillRoundedRectangle(lowerBounds, 3.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(lowerBounds, 3.0f, 1.0f);

    // Toned-down hairline vertical dividers separating Envelope, Filter/Crunch, and Master
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder.withAlpha(0.35f));
    g.drawVerticalLine(288, lowerBounds.getY() + 10.0f, lowerBounds.getBottom() - 10.0f);
    g.drawVerticalLine(566, lowerBounds.getY() + 10.0f, lowerBounds.getBottom() - 10.0f);

    // Section Titles in CairoPixel Font (identity)
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(13.0f));
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.drawText("ENVELOPE (ADSR)", 28, 296, 200, 18, juce::Justification::centredLeft);
    g.drawText("FILTER / CRUNCH", 302, 296, 200, 18, juce::Justification::centredLeft);
    g.drawText("MASTER", 580, 296, 120, 18, juce::Justification::centredLeft);

    // ==========================================
    // MASTER SECTION: REAL-TIME LED PEAK METER
    // ==========================================
    auto meterRect = juce::Rectangle<float>(672.0f, 334.0f, 14.0f, 78.0f);
    g.setColour(RetroUI::RetroLookAndFeel::wellBg);
    g.fillRoundedRectangle(meterRect, 2.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder.darker(0.1f));
    g.drawRoundedRectangle(meterRect, 2.0f, 1.0f);

    // 10 LED segments
    const int numSegments = 10;
    const float segGap = 1.5f;
    const float segH = (meterRect.getHeight() - 4.0f - (numSegments - 1) * segGap) / static_cast<float>(numSegments);
    const float segW = meterRect.getWidth() - 4.0f;
    const float segX = meterRect.getX() + 2.0f;

    // Map peak (0.0 to 1.5) to segments (0 to 10)
    int litSegments = juce::jlimit(0, numSegments, static_cast<int>(currentMeterLevel * 10.0f));

    for (int s = 0; s < numSegments; ++s)
    {
        // Draw from bottom (s=0) to top (s=9)
        int segIndexFromBottom = s;
        float segY = meterRect.getBottom() - 2.0f - (s + 1) * segH - s * segGap;
        juce::Rectangle<float> segRect(segX, segY, segW, segH);

        bool isLit = (segIndexFromBottom < litSegments);

        juce::Colour segColour = juce::Colour::fromRGB(0, 229, 163); // Green/cyan default
        if (segIndexFromBottom >= 8)
            segColour = juce::Colour::fromRGB(255, 69, 58);  // Red clipping (> 0 dB)
        else if (segIndexFromBottom >= 6)
            segColour = RetroUI::RetroLookAndFeel::warmAmber; // Amber caution (-6 dB to 0 dB)

        if (isLit)
        {
            g.setColour(segColour);
            g.fillRect(segRect);
        }
        else
        {
            g.setColour(segColour.withAlpha(0.15f));
            g.fillRect(segRect);
        }
    }

    // Clear dB Markings beside meter: only 3 readable markings (0, -6, -12)
    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.0f, juce::Font::bold));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("0", 690, 345, 24, 12, juce::Justification::centredLeft);
    g.drawText("-6", 690, 362, 24, 12, juce::Justification::centredLeft);
    g.drawText("-12", 690, 382, 24, 12, juce::Justification::centredLeft);

    // ==========================================
    // HARDWARE CRT OSCILLOSCOPE (STABILIZED TRACE)
    // ==========================================
    if (!visualizerArea.isEmpty())
    {
        auto vBounds = visualizerArea.toFloat();

        // 1. Recessed dark CRT screen housing
        g.setColour(juce::Colour::fromRGB(8, 12, 10));
        g.fillRoundedRectangle(vBounds, 3.0f);

        // 2. Bezel border
        g.setColour(RetroUI::RetroLookAndFeel::panelBorder.darker(0.2f));
        g.drawRoundedRectangle(vBounds, 3.0f, 1.0f);

        // 3. Phosphor Grid Reticle (instrumentation style)
        float midY = vBounds.getCentreY();
        float midX = vBounds.getCentreX();

        // Subtle horizontal zero centerline across the oscilloscope
        g.setColour(juce::Colour::fromRGB(32, 64, 48));
        g.drawLine(vBounds.getX(), midY, vBounds.getRight(), midY, 1.2f);

        // Center crosshairs
        g.setColour(juce::Colour::fromRGB(22, 42, 32));
        g.drawLine(midX, vBounds.getY(), midX, vBounds.getBottom(), 1.0f);

        // Reticle ticks on crosshairs (instrumentation look)
        for (float tx = vBounds.getX() + 20.0f; tx < vBounds.getRight(); tx += 20.0f)
            g.drawLine(tx, midY - 2.0f, tx, midY + 2.0f, 0.8f);
        for (float ty = vBounds.getY() + 15.0f; ty < vBounds.getBottom(); ty += 15.0f)
            g.drawLine(midX - 2.0f, ty, midX + 2.0f, ty, 0.8f);

        // Subdivisions (faint horizontal boundaries)
        g.setColour(juce::Colour::fromRGB(14, 26, 20));
        g.drawLine(vBounds.getX(), midY - vBounds.getHeight() * 0.28f, vBounds.getRight(), midY - vBounds.getHeight() * 0.28f, 0.6f);
        g.drawLine(vBounds.getX(), midY + vBounds.getHeight() * 0.28f, vBounds.getRight(), midY + vBounds.getHeight() * 0.28f, 0.6f);

        // 4. Trigger Stabilization: find first upward zero-crossing
        int triggerIdx = 0;
        for (int i = 1; i < numVisPoints - 128; ++i)
        {
            if (visPoints[static_cast<size_t>(i - 1)] <= 0.0f && visPoints[static_cast<size_t>(i)] > 0.0f)
            {
                triggerIdx = i;
                break;
            }
        }

        // 5. Instrumentation waveform trace (denser, responsive, preserving square edges)
        const int displayPoints = 128;
        juce::Path wavePath;
        float halfH = vBounds.getHeight() * 0.40f;
        float dx = vBounds.getWidth() / static_cast<float>(displayPoints - 1);

        for (int i = 0; i < displayPoints; ++i)
        {
            float x = vBounds.getX() + i * dx;
            float sample = visPoints[static_cast<size_t>((triggerIdx + i) % numVisPoints)];
            float y = midY - juce::jlimit(-1.0f, 1.0f, sample) * halfH;

            if (i == 0)
                wavePath.startNewSubPath(x, y);
            else
                wavePath.lineTo(x, y);
        }

        // Pass 1: Phosphor Bloom Glow
        g.setColour(juce::Colour::fromRGB(0, 240, 180).withAlpha(0.22f));
        g.strokePath(wavePath, juce::PathStrokeType(2.8f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));

        // Pass 2: Laser Beam Core
        g.setColour(juce::Colour::fromRGB(180, 255, 235).withAlpha(0.95f));
        g.strokePath(wavePath, juce::PathStrokeType(1.4f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));

        // 6. CRT Horizontal Scanlines
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        for (float yLine = vBounds.getY(); yLine < vBounds.getBottom(); yLine += 3.0f)
        {
            g.drawHorizontalLine(static_cast<int>(yLine), vBounds.getX(), vBounds.getRight());
        }

        // 7. Mini OUT status badge in top-right with green indicator dot
        g.setColour(juce::Colour::fromRGB(0, 240, 160));
        g.fillEllipse(vBounds.getRight() - 38.0f, vBounds.getY() + 7.5f, 4.5f, 4.5f);
        g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(8.5f, juce::Font::bold));
        g.drawText("OUT", juce::Rectangle<float>(vBounds.getRight() - 30.0f, vBounds.getY() + 4.0f, 24.0f, 11.0f).toNearestInt(), juce::Justification::centredLeft);

        // 8. Glass tube spherical vignette
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

    // OSC 1 (Left Half: x = 14 to 256, w = 242)
    osc1WaveLabel.setBounds(28, 98, 120, 14);
    osc1WaveBox.setBounds(28, 114, 214, 24);
    osc1PwLabel.setBounds(95, 148, 80, 14);
    osc1PwSlider.setBounds(95, 162, 80, 88);

    // OSC 2 (Right Half: x = 256 to 520, w = 264)
    osc2EnableToggle.setBounds(328, 72, 56, 20);
    osc2WaveLabel.setBounds(270, 98, 120, 14);
    osc2WaveBox.setBounds(270, 114, 236, 24);

    const int osc2KnobY = 162;
    const int osc2KnobW = 56;
    const int osc2KnobH = 88;
    const int osc2LblY  = 148;

    // Pitch/Tuning controls
    osc2OctLabel.setBounds(266, osc2LblY, osc2KnobW, 14);
    osc2OctSlider.setBounds(266, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2SemiLabel.setBounds(328, osc2LblY, osc2KnobW, 14);
    osc2SemiSlider.setBounds(328, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2DetuneLabel.setBounds(390, osc2LblY, osc2KnobW, 14);
    osc2DetuneSlider.setBounds(390, osc2KnobY, osc2KnobW, osc2KnobH);

    // Equal-sized Level control (matching Oct/Semi/Detune)
    oscMixLabel.setBounds(456, osc2LblY, osc2KnobW, 14);
    oscMixSlider.setBounds(456, osc2KnobY, osc2KnobW, osc2KnobH);

    // ARPEGGIATOR (x = 530, y = 54, w = 316, h = 220)
    arpModeLabel.setBounds(546, 98, 64, 22);
    arpModeBox.setBounds(616, 96, 214, 24);

    arpRateLabel.setBounds(546, 128, 64, 22);
    arpRateBox.setBounds(616, 126, 214, 24);

    // ==========================================
    // ROW 2: CONTINUOUS LOWER PLATE (y = 284, h = 244)
    // ==========================================

    // ENVELOPE (ADSR) (x = 14 to 288) - Standardized label baseline y = 324
    const int envKnobW = 58;
    const int envKnobH = 90;
    const int envKnobY = 340;
    const int envLblY  = 324;

    attackLabel.setBounds(26, envLblY, envKnobW, 14);
    attackSlider.setBounds(26, envKnobY, envKnobW, envKnobH);

    decayLabel.setBounds(90, envLblY, envKnobW, 14);
    decaySlider.setBounds(90, envKnobY, envKnobW, envKnobH);

    sustainLabel.setBounds(154, envLblY, envKnobW, 14);
    sustainSlider.setBounds(154, envKnobY, envKnobW, envKnobH);

    releaseLabel.setBounds(218, envLblY, envKnobW, 14);
    releaseSlider.setBounds(218, envKnobY, envKnobW, envKnobH);

    // FILTER / CRUNCH (x = 288 to 566) - Standardized label baseline y = 324
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

    // MASTER & SCOPE (x = 566 to 846) - Unified cluster above scope
    masterGainLabel.setBounds(598, 320, 68, 14);
    masterGainSlider.setBounds(598, 336, 68, 76);

    // Recessed widescreen CRT oscilloscope integrated beneath Volume and Level meter
    visualizerArea.setBounds(586, 422, 244, 94);
}
