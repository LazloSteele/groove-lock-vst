#pragma once
#include <JuceHeader.h>

enum class ScaleType { MINOR_PENTATONIC = 0, NATURAL_MINOR, DORIAN, BLUES, PHRYGIAN, CHROMATIC };

enum class PitchRole { ROOT, SECOND, FLAT3, FOURTH, FIFTH, FLAT5, SIXTH, FLAT7, OCTAVE, APPROACH, ANY, NONE };

static inline PitchRole parsePitchRole(const juce::String& s)
{
    if (s == "root")     return PitchRole::ROOT;
    if (s == "2")        return PitchRole::SECOND;
    if (s == "b3")       return PitchRole::FLAT3;
    if (s == "4")        return PitchRole::FOURTH;
    if (s == "5")        return PitchRole::FIFTH;
    if (s == "b5")       return PitchRole::FLAT5;
    if (s == "6")        return PitchRole::SIXTH;
    if (s == "b7")       return PitchRole::FLAT7;
    if (s == "octave")   return PitchRole::OCTAVE;
    if (s == "approach") return PitchRole::APPROACH;
    if (s == "any")      return PitchRole::ANY;
    return PitchRole::NONE;
}

// Semitone offset from root for each role (non-approach/any roles are deterministic)
static inline int semitoneForRole(PitchRole role)
{
    switch (role)
    {
        case PitchRole::ROOT:   return 0;
        case PitchRole::SECOND: return 2;
        case PitchRole::FLAT3:  return 3;
        case PitchRole::FOURTH: return 5;
        case PitchRole::FIFTH:  return 7;
        case PitchRole::FLAT5:  return 6;
        case PitchRole::SIXTH:  return 9;
        case PitchRole::FLAT7:  return 10;
        case PitchRole::OCTAVE: return 12;
        default:                return 0;
    }
}

// Semitone intervals for each scale type
static inline const int* scaleIntervals(ScaleType t, int& count)
{
    static const int minPent[]   = { 0, 3, 5, 7, 10 };
    static const int natMinor[]  = { 0, 2, 3, 5, 7, 8, 10 };
    static const int dorian[]    = { 0, 2, 3, 5, 7, 9, 10 };
    static const int blues[]     = { 0, 3, 5, 6, 7, 10 };
    static const int phrygian[]  = { 0, 1, 3, 5, 7, 8, 10 };
    static const int chromatic[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };

    switch (t)
    {
        case ScaleType::MINOR_PENTATONIC: count = 5;  return minPent;
        case ScaleType::NATURAL_MINOR:    count = 7;  return natMinor;
        case ScaleType::DORIAN:           count = 7;  return dorian;
        case ScaleType::BLUES:            count = 6;  return blues;
        case ScaleType::PHRYGIAN:         count = 7;  return phrygian;
        case ScaleType::CHROMATIC:        count = 12; return chromatic;
    }
    count = 5; return minPent;
}

// Pre-computed pitch roles for one bar (output from PhraseExpander)
struct BarPitchState
{
    PitchRole stepRoles[16];        // PitchRole per step (NONE = no bass hit)
    int       stepOctaveOffset[16]; // 0 = base octave, 1 = +1, -1 = -1
    float     deviationLevel;       // 0.0–1.0 how far this bar deviates from seed

    BarPitchState() : deviationLevel(0.f)
    {
        std::fill(stepRoles, stepRoles + 16, PitchRole::ROOT);
        std::fill(stepOctaveOffset, stepOctaveOffset + 16, 0);
    }
};

// 8-bar phrase with per-bar pitch state (double-buffered in processor)
struct ExpandedPhrase
{
    BarPitchState bars[8];
    BarPitchState bars2[8];     // bar-2 (response) phrase; valid only when hasBar2=true
    float         phraseArc[8]; // deviation level per bar
    bool          isValid;
    bool          hasBar2 = false;

    ExpandedPhrase() : isValid(false)
    {
        float arc[] = {0.0f, 0.1f, 0.2f, 0.4f, 0.3f, 0.5f, 0.7f, 0.2f};
        std::copy(arc, arc + 8, phraseArc);
    }
};

// Snap semitone offset to nearest tone in the scale
static inline int snapToScale(int semitone, ScaleType t)
{
    if (t == ScaleType::CHROMATIC) return semitone & 11;
    int count = 0;
    const int* intervals = scaleIntervals(t, count);
    int s = semitone & 11;
    int best = intervals[0], bestDist = 12;
    for (int i = 0; i < count; ++i)
    {
        int dist = std::abs(intervals[i] - s);
        if (dist > 6) dist = 12 - dist; // wrap
        if (dist < bestDist) { bestDist = dist; best = intervals[i]; }
    }
    return best;
}
