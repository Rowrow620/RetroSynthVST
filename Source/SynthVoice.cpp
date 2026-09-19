#include "SynthVoice.h"

SynthVoice::SynthVoice()
{
    adsrParams.attack  = 0.01f;
    adsrParams.decay   = 0.1f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.15f;
    adsr.setParameters(adsrParams);
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* /*sound*/, int currentPitchWheelPosition)
{
    currentNote = midiNoteNumber;
    currentVelocity = velocity;
    pitchWheel = currentPitchWheelPosition;

    osc1.reset();
    osc2.reset();

    updateFrequencies();

    adsr.setSampleRate(getSampleRate());
    adsr.setParameters(adsrParams);
    adsr.reset();
    adsr.noteOn();
}

void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{
    pitchWheel = newPitchWheelValue;
    updateFrequencies();
}

void SynthVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/)
{
}

void SynthVoice::updateParameters(
    int osc1Wave, float osc1Pw,
    bool osc2Enable, int osc2Wave, int osc2Octave, int osc2Semi, float osc2Detune,
    float oscMix,
    float attack, float decay, float sustain, float release)
{
    osc1.setWaveform(static_cast<RetroDSP::Waveform>(osc1Wave));
    osc1.setPulseWidth(osc1Pw);

    osc2Active = osc2Enable;
    osc2.setWaveform(static_cast<RetroDSP::Waveform>(osc2Wave));
    osc2Oct = osc2Octave;
    osc2Semitones = osc2Semi;
    osc2FineDetune = osc2Detune;
    mixRatio = juce::jlimit(0.0f, 1.0f, oscMix);

    adsrParams.attack  = juce::jmax(0.001f, attack);
    adsrParams.decay   = juce::jmax(0.001f, decay);
    adsrParams.sustain = juce::jlimit(0.0f, 1.0f, sustain);
    adsrParams.release = juce::jmax(0.001f, release);
    adsr.setParameters(adsrParams);

    updateFrequencies();
}

void SynthVoice::updateFrequencies()
{
    double sr = getSampleRate();
    if (sr <= 0.0)
        sr = 44100.0;

    osc1.setSampleRate(sr);
    osc2.setSampleRate(sr);

    float baseFreq = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(currentNote));

    // Pitch wheel: 0 to 16383, 8192 is center
    float pitchBendSemitones = ((pitchWheel - 8192) / 8192.0f) * pitchBendRangeSemis;
    float pitchFactor = std::pow(2.0f, pitchBendSemitones / 12.0f);

    float osc1Freq = baseFreq * pitchFactor;
    osc1.setFrequency(osc1Freq);

    // Osc 2 pitch shift = octave * 12 + semi + (cents / 100)
    float totalOsc2Semitones = (osc2Oct * 12.0f) + static_cast<float>(osc2Semitones) + (osc2FineDetune / 100.0f);
    float osc2Freq = osc1Freq * std::pow(2.0f, totalOsc2Semitones / 12.0f);
    osc2.setFrequency(osc2Freq);
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float env = adsr.getNextSample();
        float s1 = osc1.getNextSample();
        float sample = 0.0f;

        if (osc2Active)
        {
            float s2 = osc2.getNextSample();
            sample = ((s1 * (1.0f - mixRatio)) + (s2 * mixRatio)) * env * currentVelocity * 0.35f;
        }
        else
        {
            sample = s1 * env * currentVelocity * 0.35f;
        }

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
        {
            outputBuffer.addSample(ch, startSample + i, sample);
        }

        if (!adsr.isActive() || (adsrParams.sustain <= 0.0001f && env <= 0.0001f))
        {
            clearCurrentNote();
            adsr.reset();
            break;
        }
    }
}
