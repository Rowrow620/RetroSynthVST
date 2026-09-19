#include "RetroLookAndFeel.h"

namespace RetroUI
{

const juce::Colour RetroLookAndFeel::bgDark      = juce::Colour::fromRGB(18, 21, 30);
const juce::Colour RetroLookAndFeel::panelBg     = juce::Colour::fromRGB(26, 31, 44);
const juce::Colour RetroLookAndFeel::panelBorder = juce::Colour::fromRGB(45, 53, 76);
const juce::Colour RetroLookAndFeel::cyanGlow    = juce::Colour::fromRGB(0, 240, 255);
const juce::Colour RetroLookAndFeel::greenNeon   = juce::Colour::fromRGB(57, 255, 20);
const juce::Colour RetroLookAndFeel::orangeAmber = juce::Colour::fromRGB(255, 159, 28);
const juce::Colour RetroLookAndFeel::textBright  = juce::Colour::fromRGB(240, 244, 248);
const juce::Colour RetroLookAndFeel::textMuted   = juce::Colour::fromRGB(140, 150, 170);

RetroLookAndFeel::RetroLookAndFeel()
{
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxTextColourId, textBright);
    setColour(juce::ComboBox::backgroundColourId, panelBg);
    setColour(juce::ComboBox::textColourId, textBright);
    setColour(juce::ComboBox::outlineColourId, panelBorder);
    setColour(juce::PopupMenu::backgroundColourId, panelBg);
    setColour(juce::PopupMenu::textColourId, textBright);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, panelBorder);
    setColour(juce::PopupMenu::highlightedTextColourId, cyanGlow);
}

void RetroLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();

    // Background track
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(panelBorder);
    g.strokePath(bgArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active track color
    juce::Colour activeColour = cyanGlow;
    if (slider.getName().containsIgnoreCase("Env") || slider.getName().containsIgnoreCase("Attack")
        || slider.getName().containsIgnoreCase("Decay") || slider.getName().containsIgnoreCase("Sustain")
        || slider.getName().containsIgnoreCase("Release"))
    {
        activeColour = greenNeon;
    }
    else if (slider.getName().containsIgnoreCase("Bit") || slider.getName().containsIgnoreCase("Down")
             || slider.getName().containsIgnoreCase("Filter") || slider.getName().containsIgnoreCase("Cutoff")
             || slider.getName().containsIgnoreCase("Res"))
    {
        activeColour = orangeAmber;
    }

    // Active track arc
    if (slider.isEnabled())
    {
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(activeColour);
        g.strokePath(activeArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Center knob body
    float knobRadius = radius * 0.68f;
    g.setColour(panelBg.brighter(0.05f));
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    g.setColour(panelBorder);
    g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.5f);

    // Indicator pointer / dot
    juce::Path p;
    float pointerLength = knobRadius * 0.75f;
    float pointerThickness = 3.0f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -knobRadius, pointerThickness, pointerLength, 1.5f);
    p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));
    g.setColour(slider.isEnabled() ? textBright : textMuted);
    g.fillPath(p);
}

void RetroLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat();

    g.setColour(panelBg);
    g.fillRoundedRectangle(area, 4.0f);

    g.setColour(box.hasKeyboardFocus(true) ? cyanGlow : panelBorder);
    g.drawRoundedRectangle(area, 4.0f, 1.2f);

    // Retro arrow
    juce::Path arrow;
    float arrowX = width - 18.0f;
    float arrowY = height * 0.5f - 2.0f;
    arrow.addTriangle(arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour(cyanGlow);
    g.fillPath(arrow);
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
        g.setColour(panelBorder);
        g.fillRect(area);
        g.setColour(cyanGlow);
    }
    else
    {
        g.setColour(isActive ? textBright : textMuted);
    }

    auto r = area.reduced(8, 0);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

    if (isTicked)
    {
        g.setColour(cyanGlow);
        g.fillEllipse(area.getRight() - 16.0f, area.getCentreY() - 3.0f, 6.0f, 6.0f);
    }
}

void RetroLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isOn = button.getToggleState();

    g.setColour(panelBg);
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(isOn ? cyanGlow : panelBorder);
    g.drawRoundedRectangle(bounds, 3.0f, 1.2f);

    float pillW = 24.0f;
    float pillH = 12.0f;
    float pillX = bounds.getX() + 4.0f;
    float pillY = bounds.getCentreY() - (pillH * 0.5f);
    juce::Rectangle<float> pillArea(pillX, pillY, pillW, pillH);

    g.setColour(isOn ? cyanGlow.withAlpha(0.3f) : juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(pillArea, pillH * 0.5f);

    float dotDiameter = pillH - 4.0f;
    float dotX = isOn ? (pillArea.getRight() - dotDiameter - 2.0f) : (pillArea.getX() + 2.0f);
    float dotY = pillArea.getY() + 2.0f;
    g.setColour(isOn ? cyanGlow : textMuted);
    g.fillEllipse(dotX, dotY, dotDiameter, dotDiameter);

    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.setColour(isOn ? textBright : textMuted);
    auto textArea = bounds.withTrimmedLeft(pillW + 8.0f);
    g.drawFittedText(isOn ? "ON" : "OFF", textArea.toNearestInt(), juce::Justification::centredLeft, 1);
}

} // namespace RetroUI
