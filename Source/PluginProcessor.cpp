#include "PluginProcessor.h"
#include "PluginEditor.h"

SimpleSynthAudioProcessor::SimpleSynthAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Add 8 polyphonic voices
    for (int i = 0; i < 8; ++i)
        synth.addVoice(new SynthVoice());

    synth.addSound(new SynthSound());
    visualizerBuffer.fill(0.0f);
}

SimpleSynthAudioProcessor::~SimpleSynthAudioProcessor()
{
}

const juce::String SimpleSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSynthAudioProcessor::acceptsMidi() const { return true; }
bool SimpleSynthAudioProcessor::producesMidi() const { return false; }
bool SimpleSynthAudioProcessor::isMidiEffect() const { return false; }
double SimpleSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }

struct PresetDefinition
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> params;
};

static const std::vector<PresetDefinition>& getPresetList()
{
    static const std::vector<PresetDefinition> presets = {
        {
            "Authentic NES Lead",
            {
                { "osc1_wave",     0.0f },    // Pulse (PWM)
                { "osc1_pw",       0.50f },   // 50% Square
                { "osc2_enabled",  0.0f },    // Off
                { "osc2_wave",     0.0f },    // Pulse
                { "osc2_octave",   0.0f },
                { "osc2_semi",     0.0f },
                { "osc2_detune",   0.0f },
                { "osc_mix",       0.0f },
                { "env_attack",    0.001f },
                { "env_decay",     0.20f },
                { "env_sustain",   0.70f },
                { "env_release",   0.10f },
                { "filter_cutoff", 20000.0f },
                { "filter_res",    0.707f },
                { "bit_depth",     8.0f },
                { "downsample",    1.0f },
                { "arp_mode",      0.0f },    // Off
                { "arp_rate",      1.0f },    // 1/16
                { "master_gain",   -6.0f }
            }
        },
        {
            "NES Mega Man Bass",
            {
                { "osc1_wave",     1.0f },    // NES 4-bit Tri
                { "osc1_pw",       0.50f },
                { "osc2_enabled",  0.0f },    // Off
                { "osc2_wave",     1.0f },
                { "osc2_octave",   0.0f },
                { "osc2_semi",     0.0f },
                { "osc2_detune",   0.0f },
                { "osc_mix",       0.0f },
                { "env_attack",    0.001f },
                { "env_decay",     0.15f },
                { "env_sustain",   0.90f },
                { "env_release",   0.05f },
                { "filter_cutoff", 20000.0f },
                { "filter_res",    0.707f },
                { "bit_depth",     16.0f },
                { "downsample",    1.0f },
                { "arp_mode",      0.0f },    // Off
                { "arp_rate",      1.0f },    // 1/16
                { "master_gain",   -4.0f }
            }
        },
        {
            "Classic Chiptune Arp Lead",
            {
                { "osc1_wave",     0.0f },    // Pulse (PWM)
                { "osc1_pw",       0.50f },
                { "osc2_enabled",  0.0f },    // Off
                { "osc2_wave",     0.0f },
                { "osc2_octave",   0.0f },
                { "osc2_semi",     0.0f },
                { "osc2_detune",   0.0f },
                { "osc_mix",       0.0f },
                { "env_attack",    0.001f },
                { "env_decay",     0.15f },
                { "env_sustain",   0.85f },
                { "env_release",   0.10f },
                { "filter_cutoff", 20000.0f },
                { "filter_res",    0.707f },
                { "bit_depth",     16.0f },
                { "downsample",    1.0f },
                { "arp_mode",      4.0f },    // Chiptune Maj
                { "arp_rate",      2.0f },    // 1/32
                { "master_gain",   -6.0f }
            }
        },
        {
            "Retro 8-Bit Explosion SFX",
            {
                { "osc1_wave",     4.0f },    // LFSR Noise
                { "osc1_pw",       0.50f },
                { "osc2_enabled",  0.0f },    // Off
                { "osc2_wave",     4.0f },
                { "osc2_octave",   0.0f },
                { "osc2_semi",     0.0f },
                { "osc2_detune",   0.0f },
                { "osc_mix",       0.0f },
                { "env_attack",    0.001f },
                { "env_decay",     0.25f },
                { "env_sustain",   0.0f },
                { "env_release",   0.10f },
                { "filter_cutoff", 1800.0f },
                { "filter_res",    1.4f },
                { "bit_depth",     6.0f },
                { "downsample",    4.0f },
                { "arp_mode",      0.0f },    // Off
                { "arp_rate",      1.0f },    // 1/16
                { "master_gain",   -4.0f }
            }
        },
        {
            "16-Bit Genesis Lead / Brass",
            {
                { "osc1_wave",     2.0f },    // Sawtooth
                { "osc1_pw",       0.50f },
                { "osc2_enabled",  1.0f },    // On
                { "osc2_wave",     2.0f },    // Sawtooth
                { "osc2_octave",   0.0f },
                { "osc2_semi",     0.0f },
                { "osc2_detune",   12.0f },   // +12 cents detune
                { "osc_mix",       0.50f },   // 50% mix
                { "env_attack",    0.02f },
                { "env_decay",     0.30f },
                { "env_sustain",   0.75f },
                { "env_release",   0.20f },
                { "filter_cutoff", 4200.0f },
                { "filter_res",    1.2f },
                { "bit_depth",     16.0f },
                { "downsample",    1.0f },
                { "arp_mode",      0.0f },    // Off
                { "arp_rate",      1.0f },    // 1/16
                { "master_gain",   -6.0f }
            }
        }
    };
    return presets;
}

int SimpleSynthAudioProcessor::getNumPrograms()
{
    return static_cast<int>(getPresetList().size());
}

int SimpleSynthAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void SimpleSynthAudioProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms())
        return;

    currentProgram = index;
    loadPreset(index);
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
}

const juce::String SimpleSynthAudioProcessor::getProgramName(int index)
{
    const auto& presets = getPresetList();
    if (index >= 0 && index < static_cast<int>(presets.size()))
        return presets[static_cast<size_t>(index)].name;
    return {};
}

void SimpleSynthAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void SimpleSynthAudioProcessor::loadPreset(int index)
{
    const auto& presets = getPresetList();
    if (index < 0 || index >= static_cast<int>(presets.size()))
        return;

    // Hard stop all active notes and reset voices & arpeggiator on preset switch
    arpeggiator.reset();
    synth.allNotesOff(0, false);
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = synth.getVoice(i))
            v->stopNote(0.0f, false);
    }

    const auto& preset = presets[static_cast<size_t>(index)];
    for (const auto& [paramId, value] : preset.params)
    {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(paramId)))
        {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    }
}

void SimpleSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    filter.prepare(spec);
    bitCrusher.prepare(sampleRate);
    arpeggiator.prepare(sampleRate);
}

void SimpleSynthAudioProcessor::releaseResources()
{
}

bool SimpleSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void SimpleSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Fetch parameter values
    auto osc1Wave    = apvts.getRawParameterValue("osc1_wave")->load();
    auto osc1Pw      = apvts.getRawParameterValue("osc1_pw")->load();
    auto osc2Enabled = apvts.getRawParameterValue("osc2_enabled")->load() > 0.5f;
    auto osc2Wave    = apvts.getRawParameterValue("osc2_wave")->load();
    auto osc2Oct     = apvts.getRawParameterValue("osc2_octave")->load();
    auto osc2Semi    = apvts.getRawParameterValue("osc2_semi")->load();
    auto osc2Detune  = apvts.getRawParameterValue("osc2_detune")->load();
    auto oscMix      = apvts.getRawParameterValue("osc_mix")->load();

    auto attack  = apvts.getRawParameterValue("env_attack")->load();
    auto decay   = apvts.getRawParameterValue("env_decay")->load();
    auto sustain = apvts.getRawParameterValue("env_sustain")->load();
    auto release = apvts.getRawParameterValue("env_release")->load();

    auto cutoff     = apvts.getRawParameterValue("filter_cutoff")->load();
    auto resonance  = apvts.getRawParameterValue("filter_res")->load();

    auto bitDepth   = apvts.getRawParameterValue("bit_depth")->load();
    auto downsample = apvts.getRawParameterValue("downsample")->load();

    auto arpMode = static_cast<int>(apvts.getRawParameterValue("arp_mode")->load());
    auto arpRate = static_cast<int>(apvts.getRawParameterValue("arp_rate")->load());

    auto masterGainDb = apvts.getRawParameterValue("master_gain")->load();
    float masterGain = juce::Decibels::decibelsToGain(masterGainDb);

    // Update synth voices with current parameters
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->updateParameters(
                static_cast<int>(osc1Wave), osc1Pw,
                osc2Enabled, static_cast<int>(osc2Wave), static_cast<int>(osc2Oct), static_cast<int>(osc2Semi), osc2Detune,
                oscMix,
                attack, decay, sustain, release
            );
        }
    }

    // Determine host BPM and playback state
    double bpm = 120.0;
    bool isHostPlaying = false;
    if (auto* head = getPlayHead())
    {
        if (auto posInfo = head->getPosition())
        {
            if (posInfo->getBpm().hasValue())
                bpm = *posInfo->getBpm();
            isHostPlaying = posInfo->getIsPlaying();
        }
    }

    // If host transport just stopped (e.g. user pressed stop in FL Studio), cut all notes immediately
    if (wasHostPlaying && !isHostPlaying)
    {
        arpeggiator.reset();
        synth.allNotesOff(0, false);
        for (int i = 0; i < synth.getNumVoices(); ++i)
        {
            if (auto* v = synth.getVoice(i))
                v->stopNote(0.0f, false);
        }
    }
    wasHostPlaying = isHostPlaying;

    // Check for DAW All-Notes-Off / All-Sound-Off MIDI messages
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isAllNotesOff() || msg.isAllSoundOff() ||
            (msg.isController() && (msg.getControllerNumber() == 120 || msg.getControllerNumber() == 123)))
        {
            arpeggiator.reset();
            synth.allNotesOff(0, false);
            for (int i = 0; i < synth.getNumVoices(); ++i)
            {
                if (auto* v = synth.getVoice(i))
                    v->stopNote(0.0f, false);
            }
            break;
        }
    }

    // Arpeggiator intercepts MIDI note events
    arpeggiator.setMode(static_cast<RetroDSP::ArpMode>(arpMode));
    arpeggiator.setRateIndex(arpRate);
    arpeggiator.process(midiMessages, bpm, buffer.getNumSamples());

    // Synthesize voices into buffer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply Filter & Bitcrusher
    filter.setCutoffFrequency(cutoff);
    filter.setResonance(resonance);

    bitCrusher.setBitDepth(bitDepth);
    bitCrusher.setDownsampleFactor(downsample);

    auto* left = buffer.getWritePointer(0);
    auto* right = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : left;

    int writePos = visualizerWritePos.load();

    float blockPeak = 0.0f;
    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        float l = left[s];
        float r = right[s];

        filter.process(l, r);
        bitCrusher.process(l, r);

        l *= masterGain;
        r *= masterGain;

        left[s] = l;
        if (buffer.getNumChannels() > 1)
            right[s] = r;

        blockPeak = juce::jmax(blockPeak, std::abs(l), std::abs(r));

        // Push to visualizer buffer
        visualizerBuffer[static_cast<size_t>(writePos)] = (l + r) * 0.5f;
        writePos = (writePos + 1) % visualizerBufferSize;
    }

    visualizerWritePos.store(writePos);
    outputPeakLevel.store(blockPeak);
}

void SimpleSynthAudioProcessor::getVisualizerData(float* destination, int numSamples) const
{
    int currentWrite = visualizerWritePos.load();
    int readStart = (currentWrite - numSamples + visualizerBufferSize) % visualizerBufferSize;
    for (int i = 0; i < numSamples; ++i)
    {
        destination[i] = visualizerBuffer[static_cast<size_t>((readStart + i) % visualizerBufferSize)];
    }
}

bool SimpleSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SimpleSynthAudioProcessor::createEditor()
{
    return new SimpleSynthAudioProcessorEditor(*this);
}

void SimpleSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    apvts.state.setProperty("currentProgram", currentProgram, nullptr);
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SimpleSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        currentProgram = apvts.state.getProperty("currentProgram", 0);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // OSC 1
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "osc1_wave", "Osc 1 Wave",
        juce::StringArray { "Pulse (PWM)", "NES 4-bit Tri", "Sawtooth", "Sine", "LFSR Noise" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("osc1_pw", 1), "Osc 1 Pulse Width",
        juce::NormalisableRange<float>(0.05f, 0.95f, 0.01f), 0.50f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(juce::roundToInt(val * 100.0f)) + " %"; }
        )));

    // OSC 2
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("osc2_enabled", 1), "Osc 2 Enabled", true));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("osc2_wave", 1), "Osc 2 Wave",
        juce::StringArray { "Pulse (PWM)", "NES 4-bit Tri", "Sawtooth", "Sine", "LFSR Noise" }, 2));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("osc2_octave", 1), "Osc 2 Octave", -2, 2, 0,
        juce::AudioParameterIntAttributes().withStringFromValueFunction(
            [](int v, int) { return (v > 0 ? "+" : "") + juce::String(v) + " oct"; }
        )));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("osc2_semi", 1), "Osc 2 Semitone", -12, 12, 0,
        juce::AudioParameterIntAttributes().withStringFromValueFunction(
            [](int v, int) { return (v > 0 ? "+" : "") + juce::String(v) + " st"; }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("osc2_detune", 1), "Osc 2 Detune (Cents)",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.5f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { int c = juce::roundToInt(val); return (c > 0 ? "+" : "") + juce::String(c) + " ct"; }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("osc_mix", 1), "Oscillator Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(juce::roundToInt(val * 100.0f)) + " %"; }
        )));

    // ENVELOPE
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("env_attack", 1), "Attack",
        juce::NormalisableRange<float>(0.001f, 2.0f, 0.001f, 0.3f), 0.01f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) {
                if (val < 1.0f) return juce::String(juce::roundToInt(val * 1000.0f)) + " ms";
                return juce::String(val, 1) + " s";
            }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("env_decay", 1), "Decay",
        juce::NormalisableRange<float>(0.001f, 3.0f, 0.001f, 0.3f), 0.15f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) {
                if (val < 1.0f) return juce::String(juce::roundToInt(val * 1000.0f)) + " ms";
                return juce::String(val, 1) + " s";
            }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("env_sustain", 1), "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(juce::roundToInt(val * 100.0f)) + " %"; }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("env_release", 1), "Release",
        juce::NormalisableRange<float>(0.01f, 3.0f, 0.001f, 0.3f), 0.20f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) {
                if (val < 1.0f) return juce::String(juce::roundToInt(val * 1000.0f)) + " ms";
                return juce::String(val, 1) + " s";
            }
        )));

    // FILTER
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filter_cutoff", 1), "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) {
                if (val >= 1000.0f) return juce::String(val * 0.001f, 1) + " kHz";
                return juce::String(juce::roundToInt(val)) + " Hz";
            }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("filter_res", 1), "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 5.0f, 0.05f), 0.707f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(val, 2); }
        )));

    // LO-FI BITCRUSHER
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bit_depth", 1), "Bit Depth",
        juce::NormalisableRange<float>(2.0f, 16.0f, 1.0f), 16.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(juce::roundToInt(val)) + " bit"; }
        )));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("downsample", 1), "Downsample",
        juce::NormalisableRange<float>(1.0f, 32.0f, 1.0f), 1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(juce::roundToInt(val)) + "x"; }
        )));

    // ARPEGGIATOR
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("arp_mode", 1), "Arp Mode",
        juce::StringArray { "Off", "Up", "Down", "Up/Down", "Chiptune Maj", "Chiptune Min", "Chiptune Oct" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("arp_rate", 1), "Arp Rate",
        juce::StringArray { "1/8", "1/16", "1/32", "1/64" }, 1));

    // MASTER GAIN
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_gain", 1), "Master Gain",
        juce::NormalisableRange<float>(-48.0f, 6.0f, 0.5f), -6.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(
            [](float val, int) { return juce::String(val, 1) + " dB"; }
        )));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSynthAudioProcessor();
}
