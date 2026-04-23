#pragma once
#include <JuceHeader.h>
#include "PitchTypes.h"

// Velocity tiers: 0=OFF 1=GHOST 2=MED 3=FULL 4=ACCENT
enum class VelTier { OFF = 0, GHOST = 1, MED = 2, FULL = 3, ACCENT = 4 };

// Drum timing types
enum class DrumTiming { GRID, PUSH, LAY, FLAM, DRAG };

// Bass articulation / gate types
enum class BassArt { GRID, PUSH, LAY, SLIDE, BEND, STACCATO, LEGATO };

// Lock types
enum class LockType { UNISON, ALTERNATE, ANTICIPATE, FILL };

static inline DrumTiming parseDrumTiming(const juce::String& s)
{
    if (s == "push")  return DrumTiming::PUSH;
    if (s == "lay")   return DrumTiming::LAY;
    if (s == "flam")  return DrumTiming::FLAM;
    if (s == "drag")  return DrumTiming::DRAG;
    return DrumTiming::GRID;
}

static inline BassArt parseBassArt(const juce::String& s)
{
    if (s == "push")     return BassArt::PUSH;
    if (s == "lay")      return BassArt::LAY;
    if (s == "slide")    return BassArt::SLIDE;
    if (s == "bend")     return BassArt::BEND;
    if (s == "staccato") return BassArt::STACCATO;
    if (s == "legato")   return BassArt::LEGATO;
    return BassArt::GRID;
}

static inline LockType parseLockType(const juce::String& s)
{
    if (s == "alternate")  return LockType::ALTERNATE;
    if (s == "anticipate") return LockType::ANTICIPATE;
    if (s == "fill")       return LockType::FILL;
    return LockType::UNISON;
}

struct DrumRow
{
    juce::String label;
    int          steps[16]          = {};
    DrumTiming   timing[16]         = {};
};

struct BassRow
{
    juce::String label;
    int          steps[16]          = {};
    BassArt      timing[16]         = {};
};

struct LockPoint
{
    int          step               = 0;
    LockType     type               = LockType::UNISON;
    juce::String description;
};

struct PitchStepHint
{
    int       step = 0;
    PitchRole role = PitchRole::ROOT;
};

struct PitchBlock
{
    int                        densityHint           = 2;
    bool                       allowChromaticApproach = true;
    juce::Array<PitchRole>     preferredIntervals;
    juce::Array<PitchStepHint> stepHints;

    bool hasPitchData = false; // false = block was absent, use genre defaults
};

struct GrooveTemplate
{
    // meta
    juce::String name, genre, region, mood, description;
    float        tempoMin = 80.f, tempoMax = 140.f;
    float        swingPercent = 50.f;

    // pattern
    juce::OwnedArray<DrumRow>  drums;
    juce::OwnedArray<BassRow>  bass;
    juce::Array<LockPoint>     locks;
    PitchBlock                 pitch;

    bool loadFromJSON(const juce::File& file);
    bool loadFromJSON(const juce::String& jsonText);
    juce::String toJSON() const;

    // Returns the lock type at a given step, or nullopt if none
    std::optional<LockPoint> lockAt(int step) const;
};
