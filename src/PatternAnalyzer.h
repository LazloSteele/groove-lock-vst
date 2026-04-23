#pragma once
#include <JuceHeader.h>

struct DrumState
{
    int kickHits[16]  = {};
    int snareHits[16] = {};
    int hatHits[16]   = {};
    int percHits[16]  = {};
};

// Default GM drum map (user-configurable via setMapping)
struct DrumMapping
{
    juce::Array<int> kickNotes  = { 36 };
    juce::Array<int> snareNotes = { 38, 40 };
    juce::Array<int> hatNotes   = { 42, 46 };
    // everything else → perc
};

class PatternAnalyzer
{
public:
    PatternAnalyzer();

    // Call from processBlock (audio thread) — updates internal state
    void process(const juce::MidiBuffer& midi,
                 const juce::AudioPlayHead::CurrentPositionInfo& pos,
                 double sampleRate, int numSamples);

    // Returns a snapshot safe to read from the audio thread
    const DrumState& getState() const { return state; }

    void setMapping(const DrumMapping& m) { mapping = m; }

    void reset();

private:
    DrumState  state;
    DrumMapping mapping;

    int noteToCategory(int note) const;   // 0=kick 1=snare 2=hat 3=perc -1=ignore
};
