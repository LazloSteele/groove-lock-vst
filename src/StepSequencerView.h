#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "PatternAnalyzer.h"

// Renders a row-based step sequencer grid.
// mode: 0 = drums (amber), 1 = bass (blue)
class StepSequencerView : public juce::Component
{
public:
    explicit StepSequencerView(int colorMode = 0);

    void setDrumRows(const juce::OwnedArray<DrumRow>* rows);
    void setBassRows(const juce::OwnedArray<BassRow>* rows);

    // When set, overrides drumRows with live hit data (kick/snare/hat rows).
    // Pass nullptr to revert to template drum display.
    void setLiveDrumState(const DrumState* s);

    void setCurrentStep(int step);   // -1 = not playing

    // Callback when user clicks a step (row, step, new velocity tier)
    std::function<void(int row, int step, int newTier)> onStepClicked;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

private:
    int colorMode = 0;

    const juce::OwnedArray<DrumRow>* drumRows    = nullptr;
    const juce::OwnedArray<BassRow>* bassRows    = nullptr;
    const DrumState*                  liveDrumState = nullptr;

    int currentStep  = -1;
    int hoveredRow   = -1;
    int hoveredStep  = -1;

    int numRows() const;
    int getVelTier(int row, int step) const;
    juce::String getTimingLabel(int row, int step) const;

    juce::Colour cellColor(int tier) const;
    juce::Colour cellBorder(int tier) const;
    juce::Colour accentColor() const;

    void hitTest(const juce::Point<int>& pos, int& row, int& step) const;
    juce::Rectangle<int> cellBounds(int row, int step) const;

    static constexpr int kRowH    = 22;
    static constexpr int kLabelW  = 50;
    static constexpr int kPadding = 2;
};
