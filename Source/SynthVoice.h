#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthSound.h"
#include "DSP/ChiptuneOscillator.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice();

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void updateParameters(
        int osc1Wave, float osc1Pw,
        bool osc2Enable, int osc2Wave, int osc2Octave, int osc2Semi, float osc2Detune,
        float oscMix,
        float attack, float decay, float sustain, float release
    );

private:
    void updateFrequencies();

    RetroDSP::ChiptuneOscillator osc1;
    RetroDSP::ChiptuneOscillator osc2;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    int currentNote = 60;
    float currentVelocity = 0.0f;
    int pitchWheel = 8192; // Center
    float pitchBendRangeSemis = 2.0f;

    // Osc 2 tuning
    bool osc2Active = true;
    int osc2Oct = 0;
    int osc2Semitones = 0;
    float osc2FineDetune = 0.0f;
    float mixRatio = 0.0f; // 0.0 = 100% Osc1, 1.0 = 100% Osc2
};
