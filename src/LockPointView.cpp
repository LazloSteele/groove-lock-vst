#include "LockPointView.h"

LockPointView::LockPointView() {}

void LockPointView::setLocks(const juce::Array<LockPoint>* l) { locks = l; repaint(); }
void LockPointView::setCurrentStep(int s) { currentStep = s; repaint(); }

juce::Colour LockPointView::lockColor(LockType t) const
{
    switch (t)
    {
        case LockType::UNISON:     return juce::Colour(0xffff9f1c);
        case LockType::ALTERNATE:  return juce::Colour(0xff4a9eff);
        case LockType::ANTICIPATE: return juce::Colour(0xffaa77ff);
        case LockType::FILL:       return juce::Colour(0xff44cc88);
    }
    return juce::Colours::white;
}

int LockPointView::xForStep(int step) const
{
    int availW = getWidth() - kLabelW;
    float cellW = (float)availW / 16.f;
    return kLabelW + (int)((step + 0.5f) * cellW);
}

int LockPointView::stepAtX(int x) const
{
    if (x < kLabelW) return -1;
    int availW = getWidth() - kLabelW;
    float cellW = (float)availW / 16.f;
    int step = (int)((x - kLabelW) / cellW);
    return (step >= 0 && step < 16) ? step : -1;
}

void LockPointView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff121417));

    // Divider lines aligned with columns
    int availW = getWidth() - kLabelW;
    float cellW = (float)availW / 16.f;
    g.setColour(juce::Colour(0xff1e2226));
    for (int s = 0; s < 16; ++s)
    {
        int x = kLabelW + (int)(s * cellW);
        g.drawVerticalLine(x, 0, (float)getHeight());
    }

    g.setFont(10.f);
    g.setColour(juce::Colour(0xff8899aa));
    g.drawText("LOCK", 2, 0, kLabelW - 4, getHeight(), juce::Justification::centredRight);

    if (!locks) return;

    int cy = getHeight() / 2;

    for (auto& lp : *locks)
    {
        int cx = xForStep(lp.step);
        float r = (lp.step == selectedStep) ? 6.f : 4.f;
        auto col = lockColor(lp.type);

        if (lp.step == selectedStep)
        {
            g.setColour(col.withAlpha(0.25f));
            g.fillEllipse((float)cx - r * 2, (float)cy - r * 2, r * 4, r * 4);
        }

        g.setColour(col);
        g.fillEllipse((float)cx - r, (float)cy - r, r * 2, r * 2);

        if (lp.step == currentStep)
        {
            g.setColour(juce::Colours::white.withAlpha(0.4f));
            g.drawEllipse((float)cx - r - 2, (float)cy - r - 2, (r + 2) * 2, (r + 2) * 2, 1.f);
        }
    }
}

void LockPointView::mouseDown(const juce::MouseEvent& e)
{
    int step = stepAtX(e.x);
    if (step < 0) return;

    // Check if a lock exists here
    if (locks)
    {
        for (auto& lp : *locks)
        {
            if (lp.step == step)
            {
                selectedStep = (selectedStep == step) ? -1 : step;
                repaint();
                if (onLockClicked) onLockClicked(step);
                return;
            }
        }
    }
}
