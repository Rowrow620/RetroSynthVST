#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

namespace RetroDSP
{

enum class Waveform
{
    Pulse = 0,
    SteppedTriangle,
    Sawtooth,
    Sine,
    Noise
};

class ChiptuneOscillator
{
public:
    ChiptuneOscillator() = default;

    void setSampleRate(double newSampleRate) noexcept
    {
        sampleRate = (newSampleRate > 0.0) ? newSampleRate : 44100.0;
        updateIncrement();
    }

    void setFrequency(float newFrequency) noexcept
    {
        frequency = juce::jlimit(1.0f, static_cast<float>(sampleRate * 0.49), newFrequency);
        updateIncrement();
    }

    void setWaveform(Waveform newWaveform) noexcept
    {
        waveform = newWaveform;
    }

    void setPulseWidth(float newPulseWidth) noexcept
    {
        pulseWidth = juce::jlimit(0.05f, 0.95f, newPulseWidth);
    }

    void reset() noexcept
    {
        phase = 0.0f;
        lfsr = 0x7FFF; // 15-bit NES seed (all 1s)
        noiseSample = 0.0f;
        noiseCounter = 0.0f;
    }

    float getNextSample() noexcept
    {
        float output = 0.0f;

        switch (waveform)
        {
            case Waveform::Pulse:
            {
                // Classic pulse / square wave with variable duty cycle
                output = (phase < pulseWidth) ? 1.0f : -1.0f;
                break;
            }

            case Waveform::SteppedTriangle:
            {
                // Authentic NES 4-bit (16-step) quantized triangle wave
                // In the NES 2A03 APU, the sequence is 32 steps: 15 down to 0, then 0 up to 15
                // The NES hardware APU mixer provided ~+3dB hotter analog gain on the triangle
                // channel to ensure the bass cuts through against full-rail pulse leads.
                int stepIndex = static_cast<int>(phase * 32.0f) % 32;
                static constexpr int nesSteps[32] = {
                    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
                     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
                };
                int rawLevel = nesSteps[stepIndex];
                // Map 0..15 to -1.0f..+1.0f with 1.4x (+3dB) compensation
                output = ((rawLevel / 7.5f) - 1.0f) * 1.40f;
                break;
            }

            case Waveform::Sawtooth:
            {
                // Classic 16-bit arcade / SID saw
                output = (2.0f * phase) - 1.0f;
                break;
            }

            case Waveform::Sine:
            {
                output = std::sin(phase * juce::MathConstants<float>::twoPi);
                break;
            }

            case Waveform::Noise:
            {
                // NES 15-bit Linear Feedback Shift Register (LFSR)
                // Clock the LFSR at the oscillator frequency
                noiseCounter += phaseIncrement;
                if (noiseCounter >= 1.0f)
                {
                    noiseCounter -= 1.0f;

                    // NES feedback polynomial: bit 0 XOR bit 1
                    uint16_t feedback = (lfsr & 0x01) ^ ((lfsr >> 1) & 0x01);
                    lfsr = static_cast<uint16_t>((lfsr >> 1) | (feedback << 14));

                    // Output is low bit
                    noiseSample = (lfsr & 0x01) ? 1.0f : -1.0f;
                }
                output = noiseSample;
                break;
            }
        }

        // Advance phase
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;

        return output;
    }

private:
    void updateIncrement() noexcept
    {
        phaseIncrement = static_cast<float>(frequency / sampleRate);
    }

    double sampleRate = 44100.0;
    float frequency = 440.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float pulseWidth = 0.5f;
    Waveform waveform = Waveform::Pulse;

    // LFSR noise state
    uint16_t lfsr = 0x7FFF;
    float noiseSample = 0.0f;
    float noiseCounter = 0.0f;
};

} // namespace RetroDSP
