# RetroBitSynth: 8-Bit & 16-Bit Chiptune Synthesizer

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![JUCE](https://img.shields.io/badge/Framework-JUCE_7-orange.svg)](https://juce.com/)
[![Format](https://img.shields.io/badge/Format-VST3%20%7C%20Standalone-purple.svg)](https://www.steinberg.net/vst3/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-0078D6.svg)](https://microsoft.com/windows)
[![DAW](https://img.shields.io/badge/DAW-FL%20Studio%20Ready-brightgreen.svg)](https://www.image-line.com/fl-studio/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**RetroBitSynth** is an 8-voice polyphonic synthesizer plugin available in **VST3** and **Standalone** formats. It models the audio generation and conversion stages of 8-bit and 16-bit gaming sound chips (including the NES 2A03, Game Boy DMG, Commodore 64 SID, SNES SPC700, and Sega Genesis YM2612) for use in modern digital audio workstations like **FL Studio**.

Built in C++20 using the **JUCE** framework.

---

## What the Program Does

RetroBitSynth generates classic video game leads, punchy chiptune basses, metallic noise sound effects, and 16-bit polyphonic pads directly via real-time synthesis—without relying on samples or soundfonts.

### Core Features & Controls

* **Dual-Oscillator Sound Engine:**
  * **Oscillator 1 (Primary Sound Source):**
    * **Pulse (PWM):** Square and pulse wave generation with adjustable pulse width (duty cycle from 5% to 95%) for classic NES and Game Boy leads.
    * **NES 4-Bit Stepped Triangle:** 16-step quantized triangle wave replicating the Nintendo Entertainment System 2A03 APU bass generator.
    * **Sawtooth:** Sharp ramp waveform for Commodore 64 leads and 16-bit arcade chords.
    * **Sine:** Pure fundamental sine tone for deep sub-bass or mellow bells.
    * **15-Bit LFSR Noise:** Pseudo-random Linear Feedback Shift Register noise clocked to musical pitch, producing retro percussion, snares, explosions, and game sound effects.
  * **Oscillator 2 (Sub / Detune Voice):**
    * Selectable pulse, stepped triangle, saw, sine, or noise waveforms.
    * Tuning controls: Octave shift (-2 to +2 octaves), Semitone adjustment (-12 to +12 semitones), and Fine Detune (-50 to +50 cents).
    * **Osc Mix:** Crossfader blending smoothly between Oscillator 1 and Oscillator 2 for thick dual-oscillator leads and detuned unison sounds.

* **Vintage DAC Degradation (Bitcrusher):**
  * **Bit Depth Reduction:** Continuously variable quantization from 16-bit down to 2-bit to simulate lower-resolution digital converters.
  * **Sample Rate Decimation:** Zero-order hold downsampling from 1x down to 32x clock reduction, introducing authentic aliasing and lo-fi grit.

* **Arcade Chiptune Arpeggiator:**
  * **Modes:**
    * `Off`: Standard polyphonic synth playback (up to 8 simultaneous voices).
    * `Up`, `Down`, `Up/Down`: Cycles sequentially through held chord notes.
    * `Chiptune Maj`: Arpeggiates a 4-note Major triad (`Root`, `+4 semitones`, `+7 semitones`, `+12 semitones`) from a single pressed key.
    * `Chiptune Min`: Arpeggiates a 4-note Minor triad (`Root`, `+3 semitones`, `+7 semitones`, `+12 semitones`) from a single pressed key.
    * `Chiptune Oct`: Arpeggiates an octave bounce (`Root`, `+12 semitones`, `+24 semitones`) from a single pressed key.
  * **Playback Rates:** Selectable 1/8, 1/16, 1/32, and high-speed 1/64 note burst divisions.

* **Resonant Lowpass Filter & Envelopes:**
  * **2-Pole Resonant Lowpass Filter:** Cutoff sweeps from 20 Hz up to 20,000 Hz with adjustable resonance (Q up to 10.0).
  * **Filter Envelope Modulation:** Dedicated envelope depth control to dynamically open the filter cutoff on note-on triggers.
  * **Snappy ADSR Envelope:** Fast-acting amplitude envelope with attack times down to 1 ms for crisp, percussive chiptune note attacks, plus decay, sustain level, and release controls.

* **Live CRT Oscilloscope:**
  * Built-in visualizer displaying real-time audio waveforms on a 60 FPS oscilloscope display as notes and arpeggios are played.

* **Full DAW Automation & MIDI Support:**
  * Every control is exposed as an automatable VST3 parameter and responds to MIDI CC modulation and pitch bend inside FL Studio.

---

## Parameter Reference

| Parameter Name | Target Control | Range | Default | Function |
|---|---|---|---|---|
| **Osc 1 Wave** | Primary Waveform | Pulse, 4-Bit Tri, Saw, Sine, Noise | Pulse | Selects Oscillator 1 wave shape |
| **Pulse Width** | Duty Cycle | 5% – 95% | 50% | Controls pulse/square symmetry on Osc 1 |
| **Osc 2 Wave** | Secondary Waveform | Pulse, 4-Bit Tri, Saw, Sine, Noise | Saw | Selects Oscillator 2 wave shape |
| **Osc 2 Octave** | Pitch Coarse | -2 to +2 Octaves | 0 | Shifts Osc 2 pitch by whole octaves |
| **Osc 2 Semi** | Pitch Semitones | -12 to +12 Semitones | 0 | Shifts Osc 2 pitch by semitones |
| **Osc 2 Detune** | Fine Detune | -50.0 to +50.0 Cents | 0.0 | Fine-tunes Osc 2 for chorus/beating effects |
| **Osc Mix** | Voice Balance | 0.0 (Osc 1) – 1.0 (Osc 2) | 0.0 | Blends the balance between both oscillators |
| **Cutoff** | Filter Frequency | 20 Hz – 20,000 Hz | 20,000 Hz | Adjusts the lowpass filter cutoff point |
| **Resonance** | Filter Q | 0.1 – 10.0 | 0.71 | Sets the resonant peak at the cutoff frequency |
| **Env Mod** | Filter Envelope | 0.0 – 1.0 | 0.0 | Amount of ADSR envelope applied to filter cutoff |
| **Attack** | Amp Attack | 0.001s – 1.50s | 0.01s | Time to reach peak volume (down to 1ms) |
| **Decay** | Amp Decay | 0.01s – 3.00s | 0.20s | Time to drop from peak to sustain level |
| **Sustain** | Amp Sustain | 0.0 – 1.0 | 0.80 | Continuous volume while key remains held |
| **Release** | Amp Release | 0.01s – 4.00s | 0.15s | Fade-out time after key is released |
| **Bit Depth** | DAC Quantization | 2 bits – 16 bits | 16 bits | Reduces sample resolution for digital crunch |
| **Downsample** | Clock Decimation | 1x – 32x | 1x | Reduces effective sample rate to add aliasing |
| **Arp Mode** | Arpeggiator Pattern | Off, Up, Down, Up/Down, Maj, Min, Oct | Off | Selects pattern or 1-key chord arpeggiation |
| **Arp Rate** | Arpeggiator Clock | 1/8, 1/16, 1/32, 1/64 | 1/16 | Determines arpeggiation tempo division |
| **Master Vol** | Master Output | -60.0 dB to +6.0 dB | 0.0 dB | Overall plugin output volume level |

---

## Sound Design Examples

| Instrument Type | Recommended Settings |
|---|---|
| **NES Melodic Lead** | Osc 1: `Pulse (PWM)`, Pulse Width: `0.50` (Square) or `0.25` (Thin pulse). Osc Mix: `0.0`. Bit Depth: `16`, Downsample: `1`. Attack: `0.001s`, Decay: `0.2s`, Sustain: `0.7`, Release: `0.1s`. Filter: `20,000 Hz`. |
| **NES 8-Bit Bass** | Osc 1: `NES 4-bit Tri`. Osc Mix: `0.0`. Attack: `0.001s`, Decay: `0.18s`, Sustain: `0.9`, Release: `0.05s`. Filter: `3,500 Hz`. |
| **Chiptune Chord Burst** | Osc 1: `Pulse (PWM)`, Pulse Width: `0.25`. Osc Mix: `0.0`. Arp Mode: `Chiptune Maj` or `Chiptune Min`. Arp Rate: `1/32` or `1/64`. Play single notes to generate rapid retro chord bubbles. |
| **8-Bit Explosion / Snare** | Osc 1: `LFSR Noise`. Attack: `0.001s`, Decay: `0.30s`, Sustain: `0.0`, Release: `0.1s`. Filter Cutoff: `2,000 Hz`. Bit Depth: `4` or `8`. |
| **16-Bit Detuned Saw Lead** | Osc 1: `Sawtooth`. Osc 2: `Sawtooth`. Osc 2 Detune: `+12.0 cents`. Osc Mix: `0.50` (50/50 balance). Cutoff: `7,500 Hz`. Resonance: `0.40`. |

---

## Installation & Setup

### Option 1: Automated Windows Installer (Recommended)
1. Close FL Studio.
2. In this folder, right-click `install_vst3.bat` and select **"Run as administrator"**.
3. This automatically copies `RetroBitSynth.vst3` into:
   ```
   C:\Program Files\Common Files\VST3\RetroBitSynth.vst3
   ```
   and sets local write permissions so subsequent builds update seamlessly.

### Option 2: Manual VST3 Copy
Copy the built `RetroBitSynth.vst3` folder directly into your system VST3 directory:
* `C:\Program Files\Common Files\VST3\`

### Option 3: Standalone Application
Run the synthesizer directly without a DAW:
* Launch `build/SimpleSynthVST_artefacts/Release/Standalone/RetroBitSynth.exe`.
* Under **Options > Audio/MIDI Settings**, select your audio output device and any attached MIDI controller or use your computer keyboard.

---

## Using in FL Studio

1. Open **FL Studio**.
2. Navigate to **Options > Manage plugins**.
3. Ensure `C:\Program Files\Common Files\VST3` is included under **Plugin search paths**.
4. Click **Find installed plugins** at the top left.
5. Locate **RetroBitSynth** (Format: *VST3*, Type: *Synth*).
6. Click the star icon to favorite it.
7. Add it to your project via the **Channel Rack** (**+ > RetroBitSynth**).

---

## Building from Source

### Requirements
* **Windows 10 / 11 (x64)**
* **Visual Studio 2022** (with *Desktop development with C++*)
* **CMake 3.22+**
* *Git* (for fetching JUCE automatically)

### Build Commands
```powershell
# Clone the repository
git clone https://github.com/Rowrow620/RetroSynthVST.git
cd RetroSynthVST

# Generate Visual Studio project files
cmake -B build -G "Visual Studio 17 2022" -A x64

# Compile Release binaries
cmake --build build --config Release --parallel
```

Or simply run the included `build.bat` script.

Compiled outputs will be located in:
* **VST3 Plugin:** `build/SimpleSynthVST_artefacts/Release/VST3/RetroBitSynth.vst3`
* **Standalone App:** `build/SimpleSynthVST_artefacts/Release/Standalone/RetroBitSynth.exe`

---

## License

Distributed under the MIT License. See [LICENSE](LICENSE) for details.
