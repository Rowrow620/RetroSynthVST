#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace RetroUI
{

class RetroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RetroLookAndFeel();

    // Fonts
    static juce::Font getPixelFont(float height);
    static juce::Font getGeometricFont(float height, int style = juce::Font::plain);

    // Rotary Sliders
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

    // ComboBoxes & Menus
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                           bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    // Toggle Buttons
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    // Harmonized Palette Tokens (80s/90s Hardware Matte)
    static const juce::Colour bgChassis;      // #15161A Warm dark charcoal chassis
    static const juce::Colour panelBg;        // #1E2026 Hardware matte gray panel
    static const juce::Colour panelBorder;    // #2C2F38 Crisp mechanical divider/border
    static const juce::Colour wellBg;         // #121317 Recessed well
    static const juce::Colour cyanIce;        // #00E5FF Active/modulation accent
    static const juce::Colour warmAmber;      // #E58824 Filter/lo-fi crunch accent
    static const juce::Colour knobTrack;      // #30333E Neutral inactive track
    static const juce::Colour knobActive;     // #7A8296 Neutral metallic gray active arc
    static const juce::Colour textBright;     // #E5E7EB Off-white readable text
    static const juce::Colour textMuted;      // #8A91A0 Dimmed label text
    static const juce::Colour textDim;        // #4B5162 Inactive/disabled text

    // Backward-compatible aliases
    static const juce::Colour bgDark;
    static const juce::Colour cyanGlow;
    static const juce::Colour greenNeon;
    static const juce::Colour orangeAmber;
};

} // namespace RetroUI
