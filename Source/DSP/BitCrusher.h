#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

namespace RetroDSP
{

class BitCrusher
{
public:
    BitCrusher() = default;

    void prepare(double newSampleRate) noexcept
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset() noexcept
    {
        holdSampleL = 0.0f;
        holdSampleR = 0.0f;
        sampleCounter = 0.0f;
    }

    void setBitDepth(float bits) noexcept
    {
        // 2 to 16 bits
        bitDepth = juce::jlimit(2.0f, 16.0f, bits);
        quantizationLevels = std::pow(2.0f, bitDepth);
    }

    void setDownsampleFactor(float factor) noexcept
    {
        // 1.0 (no downsample) up to 32.0 (heavy downsampling)
        downsampleFactor = juce::jlimit(1.0f, 32.0f, factor);
    }

    void process(float& sampleL, float& sampleR) noexcept
    {
        sampleCounter += 1.0f;

        // Sample and hold for downsampling
        if (sampleCounter >= downsampleFactor)
        {
            sampleCounter -= downsampleFactor;

            // Apply bit reduction to the incoming samples
            holdSampleL = quantize(sampleL);
            holdSampleR = quantize(sampleR);
        }

        sampleL = holdSampleL;
        sampleR = holdSampleR;
    }

private:
    float quantize(float sample) const noexcept
    {
        if (bitDepth >= 15.9f)
            return sample; // Passthrough for full 16-bit fidelity

        // Clamp to [-1.0, 1.0]
        float clamped = juce::jlimit(-1.0f, 1.0f, sample);

        // Normalize to [0, 1], quantize to discrete steps, map back to [-1, 1]
        float normalized = (clamped + 1.0f) * 0.5f;
        float stepped = std::floor(normalized * (quantizationLevels - 1.0f) + 0.5f) / (quantizationLevels - 1.0f);
        return (stepped * 2.0f) - 1.0f;
    }

    double sampleRate = 44100.0;
    float bitDepth = 16.0f;
    float quantizationLevels = 65536.0f;
    float downsampleFactor = 1.0f;

    float holdSampleL = 0.0f;
    float holdSampleR = 0.0f;
    float sampleCounter = 0.0f;
};

} // namespace RetroDSP
