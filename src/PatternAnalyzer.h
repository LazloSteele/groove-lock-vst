#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

struct DrumState
{
    int kickHits[16]  = {};
    int snareHits[16] = {};
    int hatHits[16]   = {};
    int percHits[16]  = {};
};

struct DrumMapping
{
    juce::Array<int> kickNotes  = { 35, 36 };
    juce::Array<int> snareNotes = { 38, 39, 40 };
    juce::Array<int> hatNotes   = { 42, 44, 46 };
};

struct DrumMappingPreset
{
    juce::String name;
    DrumMapping  mapping;
};

// Built-in preset drum maps. "Custom" is not stored here — it is the state
// where the active mapping does not correspond to a named preset.
namespace DrumMappingPresets
{
    const std::vector<DrumMappingPreset>& getAll();
}

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

    // Thread-safe: message thread writes, audio thread reads via double-buffered noteMap
    void setMapping(const DrumMapping& m);

    void reset();

private:
    DrumState state;

    // Lock-free double-buffered category lookup.
    // Values: 0=kick  1=snare  2=hat  3=perc (catch-all)
    std::array<int8_t, 128> noteMaps[2];
    std::atomic<int>        activeMap { 0 };

    static void buildNoteMap(std::array<int8_t, 128>& map, const DrumMapping& m);
    int noteToCategory(int note) const;
};
