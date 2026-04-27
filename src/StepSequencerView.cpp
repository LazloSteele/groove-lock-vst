#include "StepSequencerView.h"

static const char* drumTimingSymbol(DrumTiming t)
{
    switch (t) { case DrumTiming::PUSH: return "\xe2\x86\x91"; // ↑
                 case DrumTiming::LAY:  return "\xe2\x86\x93"; // ↓
                 case DrumTiming::FLAM: return "\xe2\xab\xb6"; // ⫶
                 case DrumTiming::DRAG: return "\xe2\x89\x88"; // ≈
                 default: return ""; }
}

static const char* bassArtSymbol(BassArt a)
{
    switch (a) { case BassArt::PUSH:     return "\xe2\x86\x91"; // ↑
                 case BassArt::LAY:      return "\xe2\x86\x93"; // ↓
                 case BassArt::SLIDE:    return "\xe2\x95\xb2"; // ╲
                 case BassArt::BEND:     return "\xe2\x88\xbf"; // ∿
                 case BassArt::STACCATO: return ".";
                 case BassArt::LEGATO:   return "_";
                 default: return ""; }
}

StepSequencerView::StepSequencerView(int mode) : colorMode(mode) {}

void StepSequencerView::setDrumRows(const juce::OwnedArray<DrumRow>* r)
{
    drumRows = r; repaint();
}

void StepSequencerView::setBassRows(const juce::OwnedArray<BassRow>* r)
{
    bassRows = r; repaint();
}

void StepSequencerView::setLiveDrumState(const DrumState* s)
{
    liveDrumState = s; repaint();
}

void StepSequencerView::setCurrentStep(int s) { currentStep = s; repaint(); }

static int velToTier(int vel)
{
    if (vel <= 0)  return 0;
    if (vel <= 32) return 1;
    if (vel <= 64) return 2;
    if (vel <= 96) return 3;
    return 4;
}

static const char* kLiveRowLabels[3] = { "Kick", "Snare", "Hat" };

int StepSequencerView::numRows() const
{
    if (colorMode == 0 && liveDrumState) return 3;
    if (colorMode == 0) return drumRows ? drumRows->size() : 0;
    return bassRows ? bassRows->size() : 0;
}

int StepSequencerView::getVelTier(int row, int step) const
{
    if (colorMode == 0 && liveDrumState)
    {
        switch (row)
        {
            case 0: return velToTier(liveDrumState->kickHits[step]);
            case 1: return velToTier(liveDrumState->snareHits[step]);
            case 2: return velToTier(liveDrumState->hatHits[step]);
            default: return 0;
        }
    }
    if (colorMode == 0 && drumRows && row < drumRows->size())
        return (*drumRows)[row]->steps[step];
    if (colorMode == 1 && bassRows && row < bassRows->size())
        return (*bassRows)[row]->steps[step];
    return 0;
}

juce::String StepSequencerView::getTimingLabel(int row, int step) const
{
    if (colorMode == 0 && liveDrumState)
        return {};
    if (colorMode == 0 && drumRows && row < drumRows->size())
        return drumTimingSymbol((*drumRows)[row]->timing[step]);
    if (colorMode == 1 && bassRows && row < bassRows->size())
        return bassArtSymbol((*bassRows)[row]->timing[step]);
    return {};
}

juce::Colour StepSequencerView::cellColor(int tier) const
{
    if (colorMode == 0) // amber
    {
        switch (tier) {
            case 0: return juce::Colour(0x04ffffff);
            case 1: return juce::Colour(0xff5a4a3a);
            case 2: return juce::Colour(0xffb87830);
            case 3: return juce::Colour(0xffe89540);
            case 4: return juce::Colour(0xffff6622);
        }
    }
    else // blue
    {
        switch (tier) {
            case 0: return juce::Colour(0x04ffffff);
            case 1: return juce::Colour(0xff2a3a4a);
            case 2: return juce::Colour(0xff2868a0);
            case 3: return juce::Colour(0xff3a90d8);
            case 4: return juce::Colour(0xff22bbff);
        }
    }
    return juce::Colours::transparentBlack;
}

juce::Colour StepSequencerView::cellBorder(int tier) const
{
    if (colorMode == 0)
    {
        if (tier == 4) return juce::Colour(0xffff4400);
        if (tier == 0) return juce::Colour(0x0affffff);
        return cellColor(tier).brighter(0.15f);
    }
    else
    {
        if (tier == 4) return juce::Colour(0xff00ccff);
        if (tier == 0) return juce::Colour(0x0affffff);
        return cellColor(tier).brighter(0.15f);
    }
}

juce::Colour StepSequencerView::accentColor() const
{
    return colorMode == 0 ? juce::Colour(0xffff6622) : juce::Colour(0xff22bbff);
}

juce::Rectangle<int> StepSequencerView::cellBounds(int row, int step) const
{
    int availW = getWidth() - kLabelW;
    float cellW = (float)availW / 16.f;
    int x = kLabelW + (int)(step * cellW) + kPadding;
    int y = row * kRowH + kPadding;
    int w = (int)cellW - kPadding * 2;
    int h = kRowH - kPadding * 2;
    return { x, y, w, h };
}

void StepSequencerView::hitTest(const juce::Point<int>& pos, int& row, int& step) const
{
    row = step = -1;
    if (pos.x < kLabelW) return;
    int availW = getWidth() - kLabelW;
    float cellW = (float)availW / 16.f;
    int s = (int)((pos.x - kLabelW) / cellW);
    int r = pos.y / kRowH;
    if (s >= 0 && s < 16 && r >= 0 && r < numRows()) { step = s; row = r; }
}

void StepSequencerView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111111));

    // Step numbers header
    g.setColour(juce::Colour(0xff555555));
    g.setFont(8.f);
    for (int s = 0; s < 16; ++s)
    {
        auto r = cellBounds(0, s).withY(0).withHeight(10);
        bool isBeat = (s % 4 == 0);
        g.setColour(isBeat ? juce::Colour(0xff888888) : juce::Colour(0xff444444));
        g.drawText(juce::String(s + 1), r.translated(0, -12), juce::Justification::centred);
    }

    int rows = numRows();
    for (int row = 0; row < rows; ++row)
    {
        // Row label
        juce::String label;
        if (colorMode == 0 && liveDrumState && row < 3) label = kLiveRowLabels[row];
        else if (colorMode == 0 && drumRows)             label = (*drumRows)[row]->label;
        else if (colorMode == 1 && bassRows)             label = (*bassRows)[row]->label;
        g.setFont(10.f);
        g.setColour(colorMode == 0 ? juce::Colour(0xff999999) : juce::Colour(0xff6699cc));
        g.drawText(label, 0, row * kRowH, kLabelW - 4, kRowH, juce::Justification::centredRight);

        for (int s = 0; s < 16; ++s)
        {
            auto bounds = cellBounds(row, s);
            int tier = getVelTier(row, s);

            // Beat boundary grid lines
            bool isBeat = (s % 4 == 0);
            g.setColour(isBeat ? juce::Colour(0xff333333) : juce::Colour(0xff1e1e1e));
            g.fillRect(bounds.expanded(0));

            // Cell fill
            g.setColour(cellColor(tier));
            g.fillRect(bounds);

            // Border
            g.setColour(cellBorder(tier));
            float bw = (tier == 4) ? 2.f : 1.f;
            g.drawRect(bounds.toFloat(), bw);

            // Timing symbol
            auto sym = getTimingLabel(row, s);
            if (sym.isNotEmpty() && tier > 0)
            {
                g.setFont(7.f);
                g.setColour(juce::Colours::white.withAlpha(0.6f));
                g.drawText(sym, bounds, juce::Justification::centred);
            }

            // Playhead highlight
            if (s == currentStep)
            {
                g.setColour(juce::Colours::white.withAlpha(0.18f));
                g.fillRect(bounds.expanded(kPadding, 0));
            }
        }
    }

    // Hover highlight
    if (hoveredRow >= 0 && hoveredStep >= 0)
    {
        g.setColour(accentColor().withAlpha(0.12f));
        g.fillRect(cellBounds(hoveredRow, hoveredStep));
    }
}

void StepSequencerView::mouseMove(const juce::MouseEvent& e)
{
    hitTest(e.getPosition(), hoveredRow, hoveredStep);
    repaint();
}

void StepSequencerView::mouseDown(const juce::MouseEvent& e)
{
    int row, step;
    hitTest(e.getPosition(), row, step);
    if (row < 0 || step < 0) return;

    int cur = getVelTier(row, step);
    int next = (cur + 1) % 5;

    if (onStepClicked) onStepClicked(row, step, next);
}
