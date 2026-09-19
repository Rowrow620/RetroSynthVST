#include "PluginEditor.h"

SimpleSynthAudioProcessorEditor::SimpleSynthAudioProcessorEditor(SimpleSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&retroLookAndFeel);

    // Preset Selector
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::cyanGlow);
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
    setupComboBox(arpModeBox, arpModeLabel, "ARP MODE");

    arpRateBox.addItemList({ "1/8", "1/16", "1/32", "1/64" }, 1);
    setupComboBox(arpRateBox, arpRateLabel, "RATE");

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
    setupSlider(resSlider, resLabel, "RES", "FilterRes");

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

    setSize(840, 520);
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
    label.setFont(juce::Font(11.0f, juce::Font::bold));
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, RetroUI::RetroLookAndFeel::textMuted);
    addAndMakeVisible(label);
}

void SimpleSynthAudioProcessorEditor::setupComboBox(juce::ComboBox& box, juce::Label& label,
                                                    const juce::String& text)
{
    addAndMakeVisible(box);

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(11.0f, juce::Font::bold));
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
    audioProcessor.getVisualizerData(visPoints.data(), numVisPoints);
    if (!visualizerArea.isEmpty())
        repaint(visualizerArea);

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

    // Dark panel fill
    g.setColour(RetroUI::RetroLookAndFeel::panelBg);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Subtle border
    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawRoundedRectangle(bounds, 6.0f, 1.2f);

    // Accent line at top
    g.setColour(accentColour);
    g.fillRoundedRectangle(bounds.getX() + 8.0f, bounds.getY() + 2.0f, bounds.getWidth() - 16.0f, 2.5f, 1.0f);

    // Header label
    g.setColour(accentColour);
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.drawText(title, area.getX() + 12, area.getY() + 8, area.getWidth() - 24, 18, juce::Justification::centredLeft);
}

void SimpleSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Global background
    g.fillAll(RetroUI::RetroLookAndFeel::bgDark);

    // Header area
    g.setColour(RetroUI::RetroLookAndFeel::panelBg.darker(0.2f));
    g.fillRect(0, 0, getWidth(), 46);

    g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
    g.drawHorizontalLine(46, 0.0f, static_cast<float>(getWidth()));

    // Title & Brand
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.setColour(RetroUI::RetroLookAndFeel::cyanGlow);
    g.drawText("RETRO-BIT SYNTH", 16, 6, 260, 24, juce::Justification::centredLeft);

    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.setColour(RetroUI::RetroLookAndFeel::textMuted);
    g.drawText("8-BIT & 16-BIT CHIPTUNE ENGINE", 16, 26, 360, 16, juce::Justification::centredLeft);

    // Panels
    drawSectionPanel(g, { 12, 54, 218, 208 }, "OSC 1 (8-BIT CORE)", RetroUI::RetroLookAndFeel::cyanGlow);
    drawSectionPanel(g, { 238, 54, 348, 208 }, "OSC 2 (16-BIT DETUNE)", RetroUI::RetroLookAndFeel::cyanGlow);
    drawSectionPanel(g, { 594, 54, 234, 208 }, "CHIPTUNE ARPEGGIATOR", RetroUI::RetroLookAndFeel::greenNeon);

    drawSectionPanel(g, { 12, 270, 280, 238 }, "ENVELOPE (ADSR)", RetroUI::RetroLookAndFeel::greenNeon);
    drawSectionPanel(g, { 300, 270, 150, 238 }, "FILTER", RetroUI::RetroLookAndFeel::orangeAmber);
    drawSectionPanel(g, { 458, 270, 160, 238 }, "LO-FI CRUNCH", RetroUI::RetroLookAndFeel::orangeAmber);
    drawSectionPanel(g, { 626, 270, 202, 238 }, "MASTER & SCOPE", RetroUI::RetroLookAndFeel::cyanGlow);

    // Paint CRT oscilloscope
    if (!visualizerArea.isEmpty())
    {
        auto vBounds = visualizerArea.toFloat();
        g.setColour(juce::Colour::fromRGB(10, 14, 20));
        g.fillRoundedRectangle(vBounds, 4.0f);

        g.setColour(RetroUI::RetroLookAndFeel::panelBorder);
        g.drawRoundedRectangle(vBounds, 4.0f, 1.0f);

        // Grid lines (retro CRT style)
        g.setColour(juce::Colour::fromRGB(20, 30, 42));
        g.drawHorizontalLine(static_cast<int>(vBounds.getCentreY()), vBounds.getX(), vBounds.getRight());
        g.drawVerticalLine(static_cast<int>(vBounds.getCentreX()), vBounds.getY(), vBounds.getBottom());

        // Waveform
        juce::Path wavePath;
        float midY = vBounds.getCentreY();
        float halfH = vBounds.getHeight() * 0.42f;
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

        g.setColour(RetroUI::RetroLookAndFeel::greenNeon.withAlpha(0.9f));
        g.strokePath(wavePath, juce::PathStrokeType(1.5f));
    }
}

void SimpleSynthAudioProcessorEditor::resized()
{
    // Header Preset Selector
    presetLabel.setBounds(getWidth() - 335, 11, 55, 24);
    presetBox.setBounds(getWidth() - 272, 11, 256, 24);

    // OSC 1
    osc1WaveLabel.setBounds(24, 84, 194, 16);
    osc1WaveBox.setBounds(24, 102, 194, 26);
    osc1PwLabel.setBounds(88, 140, 70, 16);
    osc1PwSlider.setBounds(88, 156, 70, 84);

    // OSC 2
    osc2EnableToggle.setBounds(516, 62, 60, 20);
    osc2WaveLabel.setBounds(250, 84, 160, 16);
    osc2WaveBox.setBounds(250, 102, 160, 26);
    oscMixLabel.setBounds(426, 84, 144, 16);
    oscMixSlider.setBounds(468, 98, 64, 76);

    int osc2KnobY = 156;
    osc2OctLabel.setBounds(252, 140, 64, 16);
    osc2OctSlider.setBounds(252, osc2KnobY, 64, 84);

    osc2SemiLabel.setBounds(324, 140, 64, 16);
    osc2SemiSlider.setBounds(324, osc2KnobY, 64, 84);

    osc2DetuneLabel.setBounds(396, 140, 64, 16);
    osc2DetuneSlider.setBounds(396, osc2KnobY, 64, 84);

    // ARPEGGIATOR
    arpModeLabel.setBounds(606, 84, 210, 16);
    arpModeBox.setBounds(606, 104, 210, 26);

    arpRateLabel.setBounds(606, 146, 210, 16);
    arpRateBox.setBounds(606, 166, 210, 26);

    // ENVELOPE (ADSR)
    int envKnobW = 62;
    int envKnobH = 86;
    int envY = 320;

    attackLabel.setBounds(20, 302, envKnobW, 16);
    attackSlider.setBounds(20, envY, envKnobW, envKnobH);

    decayLabel.setBounds(86, 302, envKnobW, 16);
    decaySlider.setBounds(86, envY, envKnobW, envKnobH);

    sustainLabel.setBounds(152, 302, envKnobW, 16);
    sustainSlider.setBounds(152, envY, envKnobW, envKnobH);

    releaseLabel.setBounds(218, 302, envKnobW, 16);
    releaseSlider.setBounds(218, envY, envKnobW, envKnobH);

    // FILTER
    cutoffLabel.setBounds(310, 302, 60, 16);
    cutoffSlider.setBounds(310, envY, 60, envKnobH);

    resLabel.setBounds(378, 302, 60, 16);
    resSlider.setBounds(378, envY, 60, envKnobH);

    // LO-FI CRUNCH
    bitDepthLabel.setBounds(468, 302, 64, 16);
    bitDepthSlider.setBounds(468, envY, 64, envKnobH);

    downsampleLabel.setBounds(542, 302, 64, 16);
    downsampleSlider.setBounds(542, envY, 64, envKnobH);

    // MASTER & VISUALIZER
    masterGainLabel.setBounds(640, 302, 64, 16);
    masterGainSlider.setBounds(640, envY, 64, envKnobH);

    visualizerArea.setBounds(714, 306, 102, 98);
}
