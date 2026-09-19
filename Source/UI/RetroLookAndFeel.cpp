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
const juce::Colour RetroLookAndFeel::warmAmber   = juce::Colour::fromRGB(215, 126, 32); // #D77E20 Refined matte amber
const juce::Colour RetroLookAndFeel::knobTrack   = juce::Colour::fromRGB(32, 34, 41);   // #202229 Darker recessed track
const juce::Colour RetroLookAndFeel::knobActive  = juce::Colour::fromRGB(104, 112, 128);// #687080 Calm metallic gray
const juce::Colour RetroLookAndFeel::textBright  = juce::Colour::fromRGB(255, 255, 255);// #FFFFFF Pure readable white
const juce::Colour RetroLookAndFeel::textMuted   = juce::Colour::fromRGB(186, 194, 208);// #BAC2D0 ~15% brighter silver
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

    // Labels
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

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

void RetroLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.45f;
        const juce::Font font(getLabelFont(label));

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        // Border ONLY on hover or focus - never by default!
        if (label.hasKeyboardFocus(true) || label.isMouseOver())
        {
            g.setColour(panelBorder.brighter(0.35f));
            g.drawRoundedRectangle(label.getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
        }

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());
    }
    else if (label.isEnabled())
    {
        g.setColour(cyanIce.withAlpha(0.85f));
        g.drawRoundedRectangle(label.getLocalBounds().toFloat().reduced(0.5f), 2.0f, 1.0f);
    }
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

    // Interactive states
    bool isHovered  = slider.isMouseOverOrDragging();
    bool isDragging = slider.isMouseButtonDown();
    bool isFocused  = slider.hasKeyboardFocus(true);

    // Focus outline
    if (isFocused && slider.isEnabled())
    {
        g.setColour(cyanIce.withAlpha(0.65f));
        g.drawEllipse(bounds.reduced(1.0f), 1.0f);
    }

    // 1. Dark, recessed inactive background track ring
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(knobTrack);
    g.strokePath(bgArc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Determine unipolar vs bipolar parameter logic
    const auto name = slider.getName();
    bool isBipolar = name.containsIgnoreCase("Oct") ||
                     name.containsIgnoreCase("Semi") ||
                     name.containsIgnoreCase("Detune");

    float toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Accent color & stroke weight:
    // Orange Filter/Crunch is thinner (2.2f) and slightly less saturated so the white pointer dominates
    juce::Colour activeColour = knobActive;
    float arcStroke = 2.8f;

    if (name.containsIgnoreCase("Filter") || name.containsIgnoreCase("Cutoff")
        || name.containsIgnoreCase("Res") || name.containsIgnoreCase("Bit")
        || name.containsIgnoreCase("Down") || name.containsIgnoreCase("Crunch"))
    {
        activeColour = warmAmber.withAlpha(0.80f);
        arcStroke = 2.2f;
    }
    else if (name.containsIgnoreCase("Mix") || name.containsIgnoreCase("Level")
             || name.containsIgnoreCase("Pw") || name.containsIgnoreCase("Width"))
    {
        activeColour = cyanIce;
        arcStroke = 2.8f;
    }

    // Active drag boosts brightness
    if (isDragging)
    {
        activeColour = activeColour.brighter(0.35f);
        arcStroke += 0.4f;
    }

    // 3. Top-Center Detent Marker for Bipolar Controls (Octave, Semitone, Fine Tune)
    if (isBipolar)
    {
        float midAngle = (rotaryStartAngle + rotaryEndAngle) * 0.5f;
        bool isZero = std::abs(toAngle - midAngle) < 0.035f;

        // Prominent top-center notch emphasizing 0
        g.setColour(isZero ? juce::Colours::white.withAlpha(0.95f) : panelBorder.brighter(0.55f));
        g.drawLine(centreX, centreY - trackRadius - 5.0f, centreX, centreY - trackRadius + 2.0f, 2.0f);
    }

    // 4. Draw active arc track with secondary visual prominence
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
            g.strokePath(activeArc, juce::PathStrokeType(arcStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // 5. Center knob body (hardware cap with concentric groove and machined center cap)
    float knobRadius = radius * 0.70f;

    // Drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(centreX - knobRadius - 1.0f, centreY - knobRadius + 1.0f, (knobRadius + 1.0f) * 2.0f, (knobRadius + 1.0f) * 2.0f);

    if (slider.isEnabled())
    {
        // Cylindrical gradient body
        juce::ColourGradient grad(panelBg.brighter(0.06f), centreX, centreY - knobRadius,
                                  panelBg.darker(0.35f), centreX, centreY + knobRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Machined bevel rim (brightens on hover)
        g.setColour(isHovered ? panelBorder.brighter(0.40f) : panelBorder.brighter(0.12f));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

        // Recessed concentric groove
        float grooveRadius = knobRadius * 0.65f;
        g.setColour(juce::Colour::fromRGB(14, 15, 18));
        g.drawEllipse(centreX - grooveRadius, centreY - grooveRadius, grooveRadius * 2.0f, grooveRadius * 2.0f, 1.0f);

        // Machined center cap disc
        float capRadius = knobRadius * 0.38f;
        juce::ColourGradient capGrad(panelBg.darker(0.10f), centreX, centreY - capRadius,
                                     panelBg.darker(0.38f), centreX, centreY + capRadius, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        g.setColour(panelBorder.brighter(0.08f));
        g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 0.8f);

        // If Level knob: cyan accent ring on cap to separate gain from pitch
        if (name.containsIgnoreCase("Mix") || name.containsIgnoreCase("Level"))
        {
            g.setColour(cyanIce.withAlpha(0.60f));
            g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);
        }
    }
    else
    {
        g.setColour(panelBg.darker(0.3f));
        g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
        g.setColour(panelBorder.darker(0.2f));
        g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);
    }

    // 6. High-precision pointer line - BRIGHTEST element on the knob
    juce::Path p;
    float pointerInner = knobRadius * 0.32f;
    float pointerOuter = knobRadius * 0.88f;
    float pointerThick = 2.4f;

    p.addRoundedRectangle(-pointerThick * 0.5f, -pointerOuter, pointerThick, pointerOuter - pointerInner, 1.2f);
    p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centreX, centreY));

    g.setColour(slider.isEnabled() ? juce::Colours::white : textDim);
    g.fillPath(p);
}

void RetroLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto area = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);
    float alpha = box.isEnabled() ? 1.0f : 0.35f;

    // Recessed dark well
    g.setColour(wellBg.withMultipliedAlpha(alpha));
    g.fillRoundedRectangle(area, 3.0f);

    // Border with hover, active, and keyboard focus states
    if (!box.isEnabled())
    {
        g.setColour(panelBorder.withMultipliedAlpha(0.35f));
    }
    else if (box.hasKeyboardFocus(true))
    {
        g.setColour(cyanIce.withAlpha(0.85f)); // Thin cyan outline on focus
    }
    else if (box.isMouseOver() || isButtonDown)
    {
        g.setColour(panelBorder.brighter(0.45f)); // Brighter border on hover
    }
    else
    {
        g.setColour(panelBorder);
    }
    g.drawRoundedRectangle(area, 3.0f, 1.0f);

    // Clean chevron arrow
    juce::Path arrow;
    float arrowX = width - 16.0f;
    float arrowY = height * 0.5f - 2.0f;
    arrow.startNewSubPath(arrowX, arrowY);
    arrow.lineTo(arrowX + 4.5f, arrowY + 4.5f);
    arrow.lineTo(arrowX + 9.0f, arrowY);

    g.setColour((box.isEnabled() ? cyanIce : textDim).withMultipliedAlpha(alpha));
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
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
