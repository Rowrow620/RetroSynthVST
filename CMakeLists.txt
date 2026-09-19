cmake_minimum_required(VERSION 3.22)

project(SimpleSynthVST VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable MSVC multi-threaded compilation
if (MSVC)
    add_compile_options(/MP /W4 /permissive-)
endif()

# Fetch JUCE via FetchContent
include(FetchContent)

message(STATUS "Fetching JUCE 7.0.12...")
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(juce)

# Create the Audio Plugin target
juce_add_plugin(SimpleSynthVST
    COMPANY_NAME "RetroDev"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_MANUFACTURER_CODE "Rtro"
    PLUGIN_CODE "S816"
    FORMATS VST3 Standalone
    PRODUCT_NAME "RetroBitSynth"
    DESCRIPTION "8-bit & 16-bit Chiptune Synthesizer for FL Studio"
)

# Source files
target_sources(SimpleSynthVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SynthVoice.cpp
        Source/SynthVoice.h
        Source/SynthSound.h
        Source/DSP/ChiptuneOscillator.h
        Source/DSP/BitCrusher.h
        Source/DSP/RetroFilter.h
        Source/DSP/Arpeggiator.h
        Source/UI/RetroLookAndFeel.cpp
        Source/UI/RetroLookAndFeel.h
)

target_include_directories(SimpleSynthVST PRIVATE Source)

target_compile_definitions(SimpleSynthVST
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE modules
target_link_libraries(SimpleSynthVST
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(SimpleSynthVST)
