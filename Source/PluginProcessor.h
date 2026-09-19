#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SynthSound.h"
#include "SynthVoice.h"
#include "DSP/RetroFilter.h"
#include "DSP/BitCrusher.h"
#include "DSP/Arpeggiator.h"

class SimpleSynthAudioProcessor : public juce::AudioProcessor
{
public:
    SimpleSynthAudioProcessor();
    ~SimpleSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void loadPreset(int index);

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Waveform visualization circular buffer
    static constexpr int visualizerBufferSize = 512;
    void getVisualizerData(float* destination, int numSamples) const;

    int getArpStep() const { return arpeggiator.getCurrentStepIndex(); }
    bool isArpActive() const { return arpeggiator.isArpActive(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    int currentProgram { 0 };
    bool wasHostPlaying { false };

    juce::AudioProcessorValueTreeState apvts;
    juce::Synthesiser synth;

    RetroDSP::RetroFilter filter;
    RetroDSP::BitCrusher bitCrusher;
    RetroDSP::Arpeggiator arpeggiator;

    // Ring buffer for visualizer
    std::array<float, visualizerBufferSize> visualizerBuffer {};
    std::atomic<int> visualizerWritePos { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleSynthAudioProcessor)
};
