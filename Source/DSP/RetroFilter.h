#pragma once

#include <juce_dsp/juce_dsp.h>

namespace RetroDSP
{

class RetroFilter
{
public:
    RetroFilter()
    {
        filterL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filterR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        filterL.prepare(spec);
        filterR.prepare(spec);
        reset();
    }

    void reset()
    {
        filterL.reset();
        filterR.reset();
    }

    void setCutoffFrequency(float frequencyHz)
    {
        filterL.setCutoffFrequency(frequencyHz);
        filterR.setCutoffFrequency(frequencyHz);
    }

    void setResonance(float resonanceQ)
    {
        filterL.setResonance(resonanceQ);
        filterR.setResonance(resonanceQ);
    }

    void process(float& sampleL, float& sampleR) noexcept
    {
        sampleL = filterL.processSample(0, sampleL);
        sampleR = filterR.processSample(0, sampleR);
    }

private:
    juce::dsp::StateVariableTPTFilter<float> filterL;
    juce::dsp::StateVariableTPTFilter<float> filterR;
};

} // namespace RetroDSP
