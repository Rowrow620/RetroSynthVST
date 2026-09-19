#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <algorithm>

namespace RetroDSP
{

enum class ArpMode
{
    Off = 0,
    Up,
    Down,
    UpDown,
    ChiptuneMajor,
    ChiptuneMinor,
    ChiptuneOctave
};

class Arpeggiator
{
public:
    Arpeggiator() = default;

    void prepare(double newSampleRate)
    {
        sampleRate = (newSampleRate > 0.0) ? newSampleRate : 44100.0;
        reset();
    }

    void reset()
    {
        heldNotes.clear();
        currentArpNote = -1;
        stepIndex = 0;
        sampleCounter = 0;
        goingUp = true;
    }

    void setMode(ArpMode newMode)
    {
        if (mode != newMode)
        {
            mode = newMode;
            stepIndex = 0;
        }
    }

    void setRateIndex(int index)
    {
        rateIndex = juce::jlimit(0, 3, index);
    }

    void process(juce::MidiBuffer& midiMessages, double bpm, int numSamples)
    {
        if (mode == ArpMode::Off)
        {
            // If an arp note was generated previously and is still ringing, turn it off immediately!
            if (currentArpNote >= 0)
            {
                midiMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote), 0);
                currentArpNote = -1;
            }
            heldNotes.clear();
            return;
        }

        // Collect incoming MIDI note events to update held notes list
        juce::MidiBuffer outgoingMessages;

        for (const auto metadata : midiMessages)
        {
            auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                addHeldNote(msg.getNoteNumber(), msg.getVelocity());
            }
            else if (msg.isNoteOff())
            {
                removeHeldNote(msg.getNoteNumber());
            }
            else if (msg.isAllNotesOff() || msg.isAllSoundOff() ||
                     (msg.isController() && (msg.getControllerNumber() == 120 || msg.getControllerNumber() == 123)))
            {
                heldNotes.clear();
                if (currentArpNote >= 0)
                {
                    outgoingMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote), metadata.samplePosition);
                    currentArpNote = -1;
                }
                outgoingMessages.addEvent(msg, metadata.samplePosition);
            }
            else
            {
                // Pass through other messages (pitch wheel, mod wheel, CC)
                outgoingMessages.addEvent(msg, metadata.samplePosition);
            }
        }

        if (heldNotes.empty())
        {
            if (currentArpNote >= 0)
            {
                outgoingMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote), 0);
                currentArpNote = -1;
            }
            midiMessages.swapWith(outgoingMessages);
            return;
        }

        // Calculate step duration in samples based on BPM
        double safeBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : 120.0;
        double quarterNoteSamples = (sampleRate * 60.0) / safeBpm;

        double stepSamples = quarterNoteSamples / 4.0; // default 1/16
        switch (rateIndex)
        {
            case 0: stepSamples = quarterNoteSamples / 2.0; break;  // 1/8
            case 1: stepSamples = quarterNoteSamples / 4.0; break;  // 1/16
            case 2: stepSamples = quarterNoteSamples / 8.0; break;  // 1/32
            case 3: stepSamples = quarterNoteSamples / 16.0; break; // 1/64 (classic chiptune buzz)
            default: break;
        }

        int stepSamplesInt = juce::jmax(32, static_cast<int>(stepSamples));

        for (int sample = 0; sample < numSamples; ++sample)
        {
            if (sampleCounter >= stepSamplesInt || currentArpNote < 0)
            {
                sampleCounter = 0;

                // Stop previous note
                if (currentArpNote >= 0)
                {
                    outgoingMessages.addEvent(juce::MidiMessage::noteOff(1, currentArpNote), sample);
                    currentArpNote = -1;
                }

                // Pick next note
                int nextNote = getNextNote();
                if (nextNote >= 0 && nextNote <= 127)
                {
                    uint8_t vel = heldNotes.empty() ? 100 : heldNotes.back().velocity;
                    outgoingMessages.addEvent(juce::MidiMessage::noteOn(1, nextNote, vel), sample);
                    currentArpNote = nextNote;
                }
            }

            ++sampleCounter;
        }

        midiMessages.swapWith(outgoingMessages);
    }

private:
    struct NoteInfo
    {
        int noteNumber;
        uint8_t velocity;
    };

    void addHeldNote(int note, uint8_t velocity)
    {
        removeHeldNote(note);
        heldNotes.push_back({ note, velocity });
        // Sort held notes by pitch for clean Up/Down arps
        std::sort(heldNotes.begin(), heldNotes.end(), [](const NoteInfo& a, const NoteInfo& b) {
            return a.noteNumber < b.noteNumber;
        });
    }

    void removeHeldNote(int note)
    {
        heldNotes.erase(
            std::remove_if(heldNotes.begin(), heldNotes.end(),
                           [note](const NoteInfo& n) { return n.noteNumber == note; }),
            heldNotes.end());
    }

    int getNextNote()
    {
        if (heldNotes.empty())
            return -1;

        int numHeld = static_cast<int>(heldNotes.size());

        switch (mode)
        {
            case ArpMode::Up:
            {
                stepIndex = (stepIndex + 1) % numHeld;
                return heldNotes[static_cast<size_t>(stepIndex)].noteNumber;
            }

            case ArpMode::Down:
            {
                stepIndex = (stepIndex - 1 + numHeld) % numHeld;
                return heldNotes[static_cast<size_t>(stepIndex)].noteNumber;
            }

            case ArpMode::UpDown:
            {
                if (numHeld <= 1)
                {
                    stepIndex = 0;
                    return heldNotes[0].noteNumber;
                }

                if (goingUp)
                {
                    stepIndex++;
                    if (stepIndex >= numHeld - 1)
                    {
                        stepIndex = numHeld - 1;
                        goingUp = false;
                    }
                }
                else
                {
                    stepIndex--;
                    if (stepIndex <= 0)
                    {
                        stepIndex = 0;
                        goingUp = true;
                    }
                }
                return heldNotes[static_cast<size_t>(stepIndex)].noteNumber;
            }

            case ArpMode::ChiptuneMajor:
            {
                // Root, +4, +7, +12
                static constexpr int offsets[4] = { 0, 4, 7, 12 };
                stepIndex = (stepIndex + 1) % 4;
                int root = heldNotes.back().noteNumber;
                return root + offsets[stepIndex];
            }

            case ArpMode::ChiptuneMinor:
            {
                // Root, +3, +7, +12
                static constexpr int offsets[4] = { 0, 3, 7, 12 };
                stepIndex = (stepIndex + 1) % 4;
                int root = heldNotes.back().noteNumber;
                return root + offsets[stepIndex];
            }

            case ArpMode::ChiptuneOctave:
            {
                // Root, +12, +24, +12
                static constexpr int offsets[4] = { 0, 12, 24, 12 };
                stepIndex = (stepIndex + 1) % 4;
                int root = heldNotes.back().noteNumber;
                return root + offsets[stepIndex];
            }

            default:
                return heldNotes.back().noteNumber;
        }
    }

    double sampleRate = 44100.0;
    ArpMode mode = ArpMode::Off;
    int rateIndex = 1; // 1/16

    std::vector<NoteInfo> heldNotes;
    int currentArpNote = -1;
    int stepIndex = 0;
    int sampleCounter = 0;
    bool goingUp = true;
};

} // namespace RetroDSP
