#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "StepSequencerView.h"
#include "LockPointView.h"

class GrooveLockEditor : public juce::AudioProcessorEditor,
                         private juce::Timer
{
public:
    explicit GrooveLockEditor(GrooveLockProcessor&);
    ~GrooveLockEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    GrooveLockProcessor& proc;

    //=== Header ===
    juce::Label         titleLabel;
    juce::Label         presetNameLabel;
    juce::TextButton    prevButton { "<" };
    juce::TextButton    nextButton { ">" };

    //=== Main panel ===
    StepSequencerView   drumView  { 0 };
    LockPointView       lockView;
    StepSequencerView   bassView  { 1 };
    juce::Label         infoBar;

    //=== Sidebar — template browser ===
    juce::TextEditor    searchBox;
    juce::ComboBox      genreFilter;
    juce::ListBox       templateList;
    juce::StringArray   listItems;

    //=== Sidebar — global controls ===
    juce::Slider  swingKnob, humanizeKnob, velOffKnob,
                  timingOffKnob, gateScaleKnob, glideKnob;
    juce::Label   swingLabel, humanizeLabel, velOffLabel,
                  timingOffLabel, gateScaleLabel, glideLabel;

    //=== Sidebar — Density/Tension XY pad ===
    class DensityTensionPad;
    std::unique_ptr<DensityTensionPad> xyPad;
    juce::Label         xyCoordLabel;
    juce::ComboBox      regenModeBox;
    juce::TextButton    regenButton { "Regen" };

    //=== Sidebar — Pitch panel ===
    juce::ToggleButton  pitchEnabledToggle  { "Melody" };
    juce::ComboBox      pitchRootBox;
    juce::ComboBox      pitchScaleBox;
    juce::Slider        pitchDensitySlider;
    juce::Label         pitchDensityLabel;
    juce::ToggleButton  pitchChromaticToggle { "Approach Notes" };
    juce::TextButton    octaveDownButton    { "Oct -" };
    juce::TextButton    octaveUpButton      { "Oct +" };
    juce::Label         octaveDisplayLabel;
    int                 currentOctave = 2;

    //=== Transport — phrase bar indicator ===
    class PhraseBarIndicator;
    std::unique_ptr<PhraseBarIndicator> phraseBarIndicator;

    //=== Sidebar — I/O config ===
    juce::ToggleButton inputModeToggle  { "Live Drums" };
    juce::TextButton   drumConfigButton { u8"⚙" }; // gear icon — opens drum map
    class DrumMapPanel;
    std::unique_ptr<DrumMapPanel> drumMapPanel;
    juce::TextButton   recordButton { "Record Pattern" };
    juce::ComboBox     outputChannelBox;
    juce::ComboBox     rootNoteBox;
    juce::TextButton   panicButton { "PANIC" };

    //=== Transport ===
    juce::TextButton   playButton { "PLAY" };
    juce::Label        tempoLabel;
    juce::Array<juce::DrawableRectangle*> stepDots;

    //=== Internal ===
    juce::AudioPlayHead::CurrentPositionInfo lastPos;

    void timerCallback() override;
    void refreshFromTemplate();
    void rebuildTemplateList();
    void setupKnob(juce::Slider& k, juce::Label& l, const juce::String& name,
                   double lo, double hi, double def, const juce::String& suffix = {});

    class TemplateListModel;
    std::unique_ptr<TemplateListModel> listModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveLockEditor)
};
