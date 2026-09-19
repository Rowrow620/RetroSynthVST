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

    // Harmonized Palette Tokens
    static const juce::Colour bgChassis;      // #0C0E14 Deep base chassis
    static const juce::Colour panelBg;        // #141722 Slightly lighter panel surface
    static const juce::Colour panelBorder;    // #1F2535 Subtle inset/border
    static const juce::Colour wellBg;         // #0F121A Recessed well
    static const juce::Colour cyanIce;        // #00E5FF Primary accent
    static const juce::Colour warmAmber;      // #FF9500 Secondary tone/filter accent
    static const juce::Colour textBright;     // #E2E8F0 Active readouts and values
    static const juce::Colour textMuted;      // #78849E Dimmed parameter labels
    static const juce::Colour textDim;        // #4A546A Inactive/track background

    // Backward-compatible aliases
    static const juce::Colour bgDark;
    static const juce::Colour cyanGlow;
    static const juce::Colour greenNeon;
    static const juce::Colour orangeAmber;
};

} // namespace RetroUI
