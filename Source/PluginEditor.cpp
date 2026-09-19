#include "PluginEditor.h"

SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor(SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&retroLookAndFeel);

    // Preset Selector
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(11.0f, juce::Font::bold));
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::cyanIce);
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
    setupComboBox(osc1WaveBox, osc1WaveLabel, "WAVEFORM");

    osc2WaveBox.addItemList({ "Pulse (PWM)", "NES 4-bit Tri", "Sawtooth", "Sine", "LFSR Noise" }, 1);
    setupComboBox(osc2WaveBox, osc2WaveLabel, "WAVEFORM");

    arpModeBox.addItemList({ "Off", "Up", "Down", "Up/Down", "Chiptune Maj", "Chiptune Min", "Chiptune Oct" }, 1);
    setupComboBox(arpModeBox, arpModeLabel, "PATTERN MODE");

    arpRateBox.addItemList({ "1/8", "1/16", "1/32", "1/64" }, 1);
    setupComboBox(arpRateBox, arpRateLabel, "NOTE DIVISION");

    // Setup Sliders
    setupSlider(osc1PwSlider, osc1PwLabel, "PULSE WIDTH", "Osc1Pw");

    // OSC 2 Enable toggle
    addAndMakeVisible(osc2EnableToggle);
    osc2EnableToggle.setButtonText("ON");
    osc2EnableToggle.onClick = [this]() { updateOsc2EnabledState(); };

    setupSlider(osc2OctSlider, osc2OctLabel, "OCTAVE", "Osc2Oct");
    setupSlider(osc2SemiSlider, osc2SemiLabel, "SEMI", "Osc2Semi");
    setupSlider(osc2DetuneSlider, osc2DetuneLabel, "DETUNE", "Osc2Detune");
    setupSlider(oscMixSlider, oscMixLabel, "MIX", "OscMix");

    setupSlider(attackSlider, attackLabel, "ATTACK", "EnvAttack");
    setupSlider(decaySlider, decayLabel, "DECAY", "EnvDecay");
    setupSlider(sustainSlider, sustainLabel, "SUSTAIN", "EnvSustain");
    setupSlider(releaseSlider, releaseLabel, "RELEASE", "EnvRelease");

    setupSlider(cutoffSlider, cutoffLabel, "CUTOFF", "FilterCutoff");
    setupSlider(resSlider, resLabel, "RESONANCE", "FilterRes");

    setupSlider(bitDepthSlider, bitDepthLabel, "BITS", "BitDepth");
    setupSlider(downsampleSlider, downsampleLabel, "CRUNCH", "Downsample");

    setupSlider(masterGainSlider, masterGainLabel, "VOLUME", "MasterGain");

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
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 56, 16);
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
}

void SimpleSynthAudioProcessorEditor::timerCallback()
{
    // Retrieve latest scope buffer from processor
    audioProcessor.getVisualizerData(visPoints.data(), numVisPoints);

    if (!visualizerArea.isEmpty())
        repaint(visualizerArea.expanded(6));

    // Keep preset selector synchronized
    int currentProg = audioProcessor.getCurrentProgram();
    if (presetBox.getSelectedId() != currentProg + 1)
        presetBox.setSelectedId(currentProg + 1, juce::dontSendNotification);

    bool osc2Enabled = osc2EnableToggle.getToggleState();
    if (osc2WaveBox.isEnabled() != osc2Enabled)
        updateOsc2EnabledState();
}

void SimpleSynthAudioProcessorEditor::drawSectionPanel(juce::Graphics& g, const juce::Rectangle<int>& area,
                                                       const juce::String& title, juce::Colour accentColour)
{
    auto bounds = area.toFloat();

    // 1. Contrast-based module surface with soft corners
    g.setColour(RetroUI::RetroLookAndFeel::panelBg);
    g.fillRoundedRectangle(bounds, 7.0f);

    // 2. Subtle inset border (not neon!)
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(bounds, 7.0f, 1.0f);

    // 3. Subtle dark header well
    auto headerArea = bounds.removeFromTop(26.0f);
    g.setColour(RetroUI::RetroLookAndFeel::wellBg.withAlpha(0.65f));
    g.fillRoundedRectangle(headerArea, 7.0f);
    g.fillRect(headerArea.removeFromBottom(6.0f)); // Square off bottom corners

    // 4. Accent dot
    float dotSize = 5.0f;
    float dotX = area.getX() + 10.0f;
    float dotY = area.getY() + 13.0f - dotSize * 0.5f;
    g.setColour(accentColour);
    g.fillEllipse(dotX, dotY, dotSize, dotSize);

    // 5. Header label rendered in authentic Pixel Font
    g.setColour(RetroUI::RetroLookAndFeel::textBright);
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(12.0f));
    g.drawText(title, area.getX() + 20, area.getY() + 3, area.getWidth() - 30, 20, juce::Justification::centredLeft);
}

void SimpleSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Global chassis background
    g.fillAll(RetroUI::RetroLookAndFeel::bgChassis);

    // Top Header Banner
    g.setColour(RetroUI::RetroLookAndFeel::panelBg.darker(0.2f));
    g.fillRect(0, 0, getWidth(), 48);

    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawHorizontalLine(48, 0.0f, static_cast<float>(getWidth()));

    // Title & Brand in Pixel Font
    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(18.0f));
    g.setColour(RetroUI::RetroLookAndFeel::cyanIce);
    g.drawText("RETRO-BIT SYNTH", 16, 7, 260, 22, juce::Justification::centredLeft);

    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(10.0f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("8-BIT & 16-BIT CHIPTUNE HARDWARE MODEL", 16, 27, 340, 16, juce::Justification::centredLeft);

    // ROW 1: SOUND GENERATION (y = 56, h = 216)
    drawSectionPanel(g, { 14, 56, 206, 216 }, "OSC 1 (8-BIT)", RetroUI::RetroLookAndFeel::cyanIce);
    drawSectionPanel(g, { 228, 56, 354, 216 }, "OSC 2 (16-BIT DETUNE)", RetroUI::RetroLookAndFeel::cyanIce);
    drawSectionPanel(g, { 590, 56, 256, 216 }, "CHIPTUNE ARPEGGIATOR", RetroUI::RetroLookAndFeel::cyanIce);

    // Arpeggiator Status Card (eliminates dead space!)
    auto arpCard = juce::Rectangle<int>(604, 196, 228, 62);
    g.setColour(RetroUI::RetroLookAndFeel::wellBg);
    g.fillRoundedRectangle(arpCard.toFloat(), 5.0f);
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(arpCard.toFloat(), 5.0f, 1.0f);

    int arpModeIdx = arpModeBox.getSelectedId() - 1;
    juce::String arpDesc = "POLYPHONIC (8 VOICES)";
    juce::Colour badgeCol = RetroUI::RetroLookAndFeel::textMuted;

    if (arpModeIdx == 1) { arpDesc = "UP: [1 -> 2 -> 3 -> 4]"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }
    else if (arpModeIdx == 2) { arpDesc = "DOWN: [4 -> 3 -> 2 -> 1]"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }
    else if (arpModeIdx == 3) { arpDesc = "UP/DOWN PENDULUM"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }
    else if (arpModeIdx == 4) { arpDesc = "MAJOR: [0, +4, +7, +12]"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }
    else if (arpModeIdx == 5) { arpDesc = "MINOR: [0, +3, +7, +12]"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }
    else if (arpModeIdx == 6) { arpDesc = "OCTAVE: [0, +12, +24]"; badgeCol = RetroUI::RetroLookAndFeel::cyanIce; }

    g.setFont(RetroUI::RetroLookAndFeel::getGeometricFont(9.5f, juce::Font::bold));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("INTERVAL / STEP PREVIEW", arpCard.getX() + 8, arpCard.getY() + 7, arpCard.getWidth() - 16, 14, juce::Justification::left);

    g.setFont(RetroUI::RetroLookAndFeel::getPixelFont(11.0f));
    g.setColour(badgeCol);
    g.drawText(arpDesc, arpCard.getX() + 8, arpCard.getY() + 25, arpCard.getWidth() - 16, 24, juce::Justification::left);

    // ROW 2: SHAPING, TONE & MASTER (y = 280, h = 246)
    drawSectionPanel(g, { 14, 280, 270, 246 }, "ENVELOPE (ADSR)", RetroUI::RetroLookAndFeel::cyanIce);
    drawSectionPanel(g, { 292, 280, 142, 246 }, "FILTER", RetroUI::RetroLookAndFeel::warmAmber);
    drawSectionPanel(g, { 442, 280, 140, 246 }, "LO-FI CRUNCH", RetroUI::RetroLookAndFeel::warmAmber);
    drawSectionPanel(g, { 590, 280, 256, 246 }, "MASTER & SCOPE", RetroUI::RetroLookAndFeel::cyanIce);

    // Paint Hardware-Grade CRT Oscilloscope
    if (!visualizerArea.isEmpty())
    {
        auto vBounds = visualizerArea.toFloat();

        // 1. Recessed screen housing
        g.setColour(juce::Colour::fromRGB(8, 12, 10)); // Deep CRT phosphor black
        g.fillRoundedRectangle(vBounds, 6.0f);

        // 2. Bezel border
        g.setColour(RetroUI::RetroLookAndFeel::panelBorder.darker(0.2f));
        g.drawRoundedRectangle(vBounds, 6.0f, 1.2f);

        // 3. CRT Grid Reticle
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
        g.setColour(juce::Colour::fromRGB(0, 240, 180).withAlpha(0.22f));
        g.strokePath(wavePath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 6. Crisp Beam Core (Pass 2)
        g.setColour(juce::Colour::fromRGB(110, 255, 215).withAlpha(0.95f));
        g.strokePath(wavePath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 7. CRT Horizontal Scanlines
        g.setColour(juce::Colours::black.withAlpha(0.12f));
        for (float yLine = vBounds.getY(); yLine < vBounds.getBottom(); yLine += 3.0f)
        {
            g.drawHorizontalLine(static_cast<int>(yLine), vBounds.getX(), vBounds.getRight());
        }

        // 8. Glass tube vignette & reflection curvature
        juce::ColourGradient vignette(juce::Colours::transparentBlack, midX, midY,
                                      juce::Colour::fromRGB(2, 4, 3).withAlpha(0.40f), vBounds.getX(), vBounds.getY(), true);
        g.setGradientFill(vignette);
        g.fillRoundedRectangle(vBounds, 6.0f);
    }
}

void SimpleSynthAudioProcessorEditor::resized()
{
    // Preset dropdown in header
    presetLabel.setBounds(getWidth() - 325, 11, 55, 26);
    presetBox.setBounds(getWidth() - 264, 11, 248, 26);

    // ==========================================
    // ROW 1 (y = 56, h = 216)
    // ==========================================

    // 1. OSC 1 (14, 56, 206, 216)
    osc1WaveLabel.setBounds(26, 88, 182, 16);
    osc1WaveBox.setBounds(26, 106, 182, 26);
    osc1PwLabel.setBounds(79, 146, 76, 16);
    osc1PwSlider.setBounds(79, 164, 76, 88);

    // 2. OSC 2 (228, 56, 354, 216)
    osc2EnableToggle.setBounds(518, 60, 56, 20);
    osc2WaveLabel.setBounds(240, 88, 172, 16);
    osc2WaveBox.setBounds(240, 106, 172, 26);

    oscMixLabel.setBounds(432, 88, 134, 16);
    oscMixSlider.setBounds(468, 104, 68, 80);

    const int osc2KnobY = 164;
    const int osc2KnobW = 72;
    const int osc2KnobH = 88;
    osc2OctLabel.setBounds(240, 146, osc2KnobW, 16);
    osc2OctSlider.setBounds(240, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2SemiLabel.setBounds(318, 146, osc2KnobW, 16);
    osc2SemiSlider.setBounds(318, osc2KnobY, osc2KnobW, osc2KnobH);

    osc2DetuneLabel.setBounds(396, 146, osc2KnobW, 16);
    osc2DetuneSlider.setBounds(396, osc2KnobY, osc2KnobW, osc2KnobH);

    // 3. ARPEGGIATOR (590, 56, 256, 216)
    arpModeLabel.setBounds(604, 88, 228, 16);
    arpModeBox.setBounds(604, 106, 228, 26);

    arpRateLabel.setBounds(604, 140, 228, 16);
    arpRateBox.setBounds(604, 158, 228, 26);

    // ==========================================
    // ROW 2 (y = 280, h = 246)
    // ==========================================

    // 4. ENVELOPE (ADSR) (14, 280, 270, 246)
    const int envKnobW = 60;
    const int envKnobH = 88;
    const int envKnobY = 340;
    const int envLblY = 320;

    attackLabel.setBounds(22, envLblY, envKnobW, 16);
    attackSlider.setBounds(22, envKnobY, envKnobW, envKnobH);

    decayLabel.setBounds(88, envLblY, envKnobW, 16);
    decaySlider.setBounds(88, envKnobY, envKnobW, envKnobH);

    sustainLabel.setBounds(154, envLblY, envKnobW, 16);
    sustainSlider.setBounds(154, envKnobY, envKnobW, envKnobH);

    releaseLabel.setBounds(220, envLblY, envKnobW, 16);
    releaseSlider.setBounds(220, envKnobY, envKnobW, envKnobH);

    // 5. FILTER (292, 280, 142, 246)
    cutoffLabel.setBounds(300, envLblY, envKnobW, 16);
    cutoffSlider.setBounds(300, envKnobY, envKnobW, envKnobH);

    resLabel.setBounds(366, envLblY, envKnobW, 16);
    resSlider.setBounds(366, envKnobY, envKnobW, envKnobH);

    // 6. LO-FI CRUNCH (442, 280, 140, 246)
    bitDepthLabel.setBounds(450, envLblY, envKnobW, 16);
    bitDepthSlider.setBounds(450, envKnobY, envKnobW, envKnobH);

    downsampleLabel.setBounds(514, envLblY, envKnobW, 16);
    downsampleSlider.setBounds(514, envKnobY, envKnobW, envKnobH);

    // 7. MASTER & SCOPE (590, 280, 256, 246)
    masterGainLabel.setBounds(600, envLblY, 64, 16);
    masterGainSlider.setBounds(600, envKnobY, 64, envKnobH);

    // Expanded widescreen CRT Oscilloscope
    visualizerArea.setBounds(672, 314, 162, 198);
}
