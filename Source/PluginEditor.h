#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/RetroLookAndFeel.h"

class SimpleSynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit SimpleSynthAudioProcessorEditor(SimpleSynthAudioProcessor&);
    ~SimpleSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, const juce::String& name);
    void setupComboBox(juce::ComboBox& box, juce::Label& label, const juce::String& text);

    void drawSectionPanel(juce::Graphics& g, const juce::Rectangle<int>& area,
                          const juce::String& title, juce::Colour accentColour);

    SimpleSynthAudioProcessor& audioProcessor;
    RetroUI::RetroLookAndFeel retroLookAndFeel;

    // PRESET
    juce::ComboBox presetBox;
    juce::Label presetLabel;

    // OSC 1
    juce::ComboBox osc1WaveBox;
    juce::Label osc1WaveLabel;
    juce::Slider osc1PwSlider;
    juce::Label osc1PwLabel;

    // OSC 2
    juce::ToggleButton osc2EnableToggle;
    juce::ComboBox osc2WaveBox;
    juce::Label osc2WaveLabel;
    juce::Slider osc2OctSlider;
    juce::Label osc2OctLabel;
    juce::Slider osc2SemiSlider;
    juce::Label osc2SemiLabel;
    juce::Slider osc2DetuneSlider;
    juce::Label osc2DetuneLabel;
    juce::Slider oscMixSlider;
    juce::Label oscMixLabel;

    // ENVELOPE
    juce::Slider attackSlider;
    juce::Label attackLabel;
    juce::Slider decaySlider;
    juce::Label decayLabel;
    juce::Slider sustainSlider;
    juce::Label sustainLabel;
    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    // FILTER
    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;
    juce::Slider resSlider;
    juce::Label resLabel;

    // LO-FI BITCRUSHER
    juce::Slider bitDepthSlider;
    juce::Label bitDepthLabel;
    juce::Slider downsampleSlider;
    juce::Label downsampleLabel;

    // ARPEGGIATOR
    juce::ComboBox arpModeBox;
    juce::Label arpModeLabel;
    juce::ComboBox arpRateBox;
    juce::Label arpRateLabel;

    // MASTER
    juce::Slider masterGainSlider;
    juce::Label masterGainLabel;

    // Visualizer data
    static constexpr int numVisPoints = 256;
    std::array<float, numVisPoints> visPoints {};
    juce::Rectangle<int> visualizerArea;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void updateOsc2EnabledState();

    std::unique_ptr<ComboAttachment> osc1WaveAttach;
    std::unique_ptr<SliderAttachment> osc1PwAttach;

    std::unique_ptr<ButtonAttachment> osc2EnableAttach;
    std::unique_ptr<ComboAttachment> osc2WaveAttach;
    std::unique_ptr<SliderAttachment> osc2OctAttach;
    std::unique_ptr<SliderAttachment> osc2SemiAttach;
    std::unique_ptr<SliderAttachment> osc2DetuneAttach;
    std::unique_ptr<SliderAttachment> oscMixAttach;

    std::unique_ptr<SliderAttachment> attackAttach;
    std::unique_ptr<SliderAttachment> decayAttach;
    std::unique_ptr<SliderAttachment> sustainAttach;
    std::unique_ptr<SliderAttachment> releaseAttach;

    std::unique_ptr<SliderAttachment> cutoffAttach;
    std::unique_ptr<SliderAttachment> resAttach;

    std::unique_ptr<SliderAttachment> bitDepthAttach;
    std::unique_ptr<SliderAttachment> downsampleAttach;

    std::unique_ptr<ComboAttachment> arpModeAttach;
    std::unique_ptr<ComboAttachment> arpRateAttach;

    std::unique_ptr<SliderAttachment> masterGainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthAudioProcessorEditor)
};
