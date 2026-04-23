#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"

class LockPointView : public juce::Component
{
public:
    LockPointView();

    void setLocks(const juce::Array<LockPoint>* locks);
    void setCurrentStep(int step);

    // Called when user clicks a lock dot — receives step index
    std::function<void(int step)> onLockClicked;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    const juce::Array<LockPoint>* locks = nullptr;
    int currentStep   = -1;
    int selectedStep  = -1;

    juce::Colour lockColor(LockType t) const;
    int stepAtX(int x) const;
    int xForStep(int step) const;

    static constexpr int kLabelW = 50;
};
