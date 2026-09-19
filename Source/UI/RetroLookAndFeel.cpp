#include "RetroLookAndFeel.h"
#include "PixelFontData.h"

namespace RetroUI
{

// Harmonized Palette Tokens (80s/90s Hardware Matte)
const juce::Colour RetroLookAndFeel::bgChassis   = juce::Colour::fromRGB(21, 22, 26);   // #15161A
const juce::Colour RetroLookAndFeel::panelBg     = juce::Colour::fromRGB(30, 32, 38);   // #1E2026
const juce::Colour RetroLookAndFeel::panelBorder = juce::Colour::fromRGB(44, 47, 56);   // #2C2F38
const juce::Colour RetroLookAndFeel::wellBg      = juce::Colour::fromRGB(18, 19, 23);   // #121317
const juce::Colour RetroLookAndFeel::cyanIce     = juce::Colour::fromRGB(0, 229, 255);  // #00E5FF
const juce::Colour RetroLookAndFeel::warmAmber   = juce::Colour::fromRGB(229, 136, 36); // #E58824
const juce::Colour RetroLookAndFeel::knobTrack   = juce::Colour::fromRGB(48, 51, 62);   // #30333E
const juce::Colour RetroLookAndFeel::knobActive  = juce::Colour::fromRGB(122, 130, 150);// #7A8296
const juce::Colour RetroLookAndFeel::textBright  = juce::Colour::fromRGB(229, 231, 235);// #E5E7EB
const juce::Colour RetroLookAndFeel::textMuted   = juce::Colour::fromRGB(138, 145, 160);// #8A91A0
const juce::Colour RetroLookAndFeel::textDim     = juce::Colour::fromRGB(75, 81, 98);   // #4B5162

// Compatibility Aliases
const juce::Colour RetroLookAndFeel::bgDark      = RetroLookAndFeel::bgChassis;
const juce::Colour RetroLookAndFeel::cyanGlow    = RetroLookAndFeel::cyanIce;
const juce::Colour RetroLookAndFeel::greenNeon   = RetroLookAndFeel::cyanIce;
const juce::Colour RetroLookAndFeel::orangeAmber = RetroLookAndFeel::warmAmber;

RetroLookAndFeel::RetroLookAndFeel()
{
    // Sliders
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxTextColourId, textBright);

    // Dropdowns
    setColour(juce::ComboBox::backgroundColourId, wellBg);
    setColour(juce::ComboBox::textColourId, textBright);
    setColour(juce::ComboBox::outlineColourId, panelBorder);
    setColour(juce::ComboBox::arrowColourId, textMuted);

    // Menus
    setColour(juce::PopupMenu::backgroundColourId, panelBg);
    setColour(juce::PopupMenu::textColourId, textBright);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, panelBorder);
    setColour(juce::PopupMenu::highlightedTextColourId, cyanIce);
}

juce::Font RetroLookAndFeel::getPixelFont(float height)
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor(cairoPixelFontData, cairoPixelFontSize);
    if (typeface != nullptr)
        return juce::Font(typeface).withHeight(height);
    return juce::Font(juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain);
}

juce::Font RetroLookAndFeel::getGeometricFont(float height, int style)
{
    return juce::Font("Segoe UI", height, style);
}

juce::Label* RetroLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
    l->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    l->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    l->setColour(juce::Label::textColourId, textBright);
    l->setFont(getGeometricFont(11.0f, juce::Font::plain));
    l->setJustificationType(juce::Justification::centred);
    return l;
}

void RetroLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    const float trackRadius = radius - 3.5f;

    // 1. Inactive background track ring
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(knobTrack);
    g.strokePath(bgArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Determine unipolar vs bipolar parameter logic
    const auto name = slider.getName();
    bool isBipolar = name.containsIgnoreCase("Oct") ||
                     name.containsIgnoreCase("Semi") ||
                     name.containsIgnoreCase("Detune") ||
                     name.containsIgnoreCase("Gain") ||
                     name.containsIgnoreCase("Volume");

    float toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Accent color: Warm amber for filter/crunch, Cyan for level/pw, Neutral metallic gray for everything else!
    juce::Colour activeColour = knobActive;
    if (name.containsIgnoreCase("Filter") || name.containsIgnoreCase("Cutoff")
        || name.containsIgnoreCase("Res") || name.containsIgnoreCase("Bit")
        || name.containsIgnoreCase("Down") || name.containsIgnoreCase("Crunch"))
    {
        activeColour = warmAmber;
    }
    else if (name.containsIgnoreCase("Mix") || name.containsIgnoreCase("Level")
             || name.containsIgnoreCase("Pw") || name.containsIgnoreCase("Width"))
    {
        activeColour = cyanIce.withAlpha(0.85f);
    }

    // 3. Draw active arc track with mathematically accurate ranges
    if (slider.isEnabled())
    {
        float fromAngle = rotaryStartAngle;
        float sweepTo = toAngle;

        if (isBipolar)
        {
            float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
            if (toAngle >= midAngle)
            {
                fromAngle = midAngle;
                sweepTo = toAngle;
            }
            else
            {
                fromAngle = toAngle;
                sweepTo = midAngle;
            }
        }

        if (std::abs(sweepTo - fromAngle) > 0.005f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, fromAngle, sweepTo, true);
            g.setColour(activeColour);
            g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        else if (isBipolar)
        {
            // Draw center detent tick for bipolar zero position
            float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
            float cosA = std::sin(midAngle);
            float sinA = -std::cos(midAngle);
            g.setColour(textMuted);
            g.drawLine(centreX + cosA * (trackRadius - 4.0f), centreY + sinA * (trackRadius - 4.0f),
                       centreX + cosA * (trackRadius + 4.0f), centreY + sinA * (trackRadius + 4.0f), 1.5f);
        }
    }

    // 4. Center knob body (hardware cylindrical cap with subtle bevel)
    float knobRadius = radius * 0.70f;

    // Subtle drop shadow / rim well
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(centreX - knobRadius - 1.0f, centreY - knobRadius + 1.0f, (knobRadius + 1.0f) * 2.0f, (knobRadius + 1.0f) * 2.0f);

    if (slider.isEnabled())
    {
        // Cylindrical gradient body
        juce::ColourGradient grad(panelBg.brighter(0.12f), centreX, centreY - knobRadius,
                                  panelBg.darker(0.15f), centreX, centreY + knobRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Machined edge rim
        g.setColour(panelBorder.brighter(0.15f));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);
    }
    else
    {
        g.setColour(panelBg.darker(0.2f));
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        g.setColour(panelBorder.darker(0.2f));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);
    }

    // 5. High-precision pointer line
    juce::Path p;
    float pointerInner = knobRadius * 0.32f;
    float pointerOuter = knobRadius * 0.85f;
    float pointerThick = 2.4f;

    p.addRoundedRectangle(-pointerThick * 0.5f, -pointerOuter, pointerThick, pointerOuter - pointerInner, 1.2f);
    p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));

    g.setColour(slider.isEnabled() ? textBright : textDim);
    g.fillPath(p);
}

void RetroLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);

    // Recessed dark well
    g.setColour(wellBg);
    g.fillRoundedRectangle(area, 4.0f);

    // Border: subtle neutral grey, highlighted with primary cyan when focused
    g.setColour(box.hasKeyboardFocus(true) ? cyanIce.withAlpha(0.7f) : panelBorder);
    g.drawRoundedRectangle(area, 4.0f, 1.0f);

    // Clean chevron arrow
    juce::Path arrow;
    float arrowX = width - 16.0f;
    float arrowY = height * 0.5f - 2.0f;
    arrow.startNewSubPath(arrowX, arrowY);
    arrow.lineTo(arrowX + 4.5f, arrowY + 4.5f);
    arrow.lineTo(arrowX + 9.0f, arrowY);

    g.setColour(cyanIce.withAlpha(0.85f));
    g.strokePath(arrow, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void RetroLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(panelBg);
    g.setColour(panelBorder);
    g.drawRect(0, 0, width, height, 1);
}

void RetroLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                                        bool /*hasSubMenu*/, const juce::String& text,
                                        const juce::String& /*shortcutKeyText*/,
                                        const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/)
{
    if (isSeparator)
    {
        g.setColour(panelBorder);
        g.fillRect(area.getX(), area.getCentreY(), area.getWidth(), 1);
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour(panelBorder.brighter(0.1f));
        g.fillRect(area);
        g.setColour(cyanIce);
    }
    else
    {
        g.setColour(isActive ? textBright : textDim);
    }

    auto r = area.reduced(10, 0);
    g.setFont(getGeometricFont(12.5f, juce::Font::plain));
    g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

    if (isTicked)
    {
        g.setColour(cyanIce);
        g.fillEllipse(area.getRight() - 16.0f, area.getCentreY() - 3.0f, 6.0f, 6.0f);
    }
}

void RetroLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isOn = button.getToggleState();

    float pillW = 28.0f;
    float pillH = 14.0f;
    float pillX = bounds.getX() + 2.0f;
    float pillY = bounds.getCentreY() - (pillH * 0.5f);
    juce::Rectangle<float> pillArea(pillX, pillY, pillW, pillH);

    // Pill background
    g.setColour(isOn ? cyanIce.withAlpha(0.25f) : wellBg);
    g.fillRoundedRectangle(pillArea, pillH * 0.5f);

    // Pill border
    g.setColour(isOn ? cyanIce.withAlpha(0.6f) : panelBorder);
    g.drawRoundedRectangle(pillArea, pillH * 0.5f, 1.0f);

    // Moving thumb dot
    float dotDiameter = pillH - 4.0f;
    float dotX = isOn ? (pillArea.getRight() - dotDiameter - 2.0f) : (pillArea.getX() + 2.0f);
    float dotY = pillArea.getY() + 2.0f;

    g.setColour(isOn ? cyanIce : textMuted);
    g.fillEllipse(dotX, dotY, dotDiameter, dotDiameter);

    // Text label
    g.setFont(getGeometricFont(10.5f, juce::Font::bold));
    g.setColour(isOn ? textBright : textMuted);
    auto textArea = bounds.withTrimmedLeft(pillW + 6.0f);
    g.drawFittedText(isOn ? "ON" : "OFF", textArea.toNearestInt(), juce::Justification::centredLeft, 1);
}

} // namespace RetroUI
