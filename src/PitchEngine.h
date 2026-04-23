#pragma once
#include <JuceHeader.h>
#include "PitchTypes.h"
#include "GrooveTemplate.h"
#include "GenreProfile.h"

struct PitchEngineParams
{
    int       rootMidiNote      = 36;   // base root (e.g. 36 = C2)
    ScaleType scaleType         = ScaleType::MINOR_PENTATONIC;
    int       densityOverride   = 0;    // 0 = use template/genre hint
    bool      chromaticApproach = true;
    bool      pitchEnabled      = false;
};

class PitchEngine
{
public:
    // Pre-compute all 16 MIDI note numbers for the bar.
    // Call this at step 0 of each bar before processStep fires.
    void computeBar(const GrooveTemplate* tmpl,
                    const GenreProfile&   profile,
                    const PitchEngineParams& params);

    // Returns MIDI note for this step, or params.rootMidiNote if pitch is disabled
    // or the step has no active bass hit.
    int getNoteForStep(int step) const;

    void reset();

private:
    int  barNotes[16];
    bool ready = false;

    juce::Random random;

    // Resolve which pitch role applies to a step
    PitchRole roleForStep(int step,
                          const GrooveTemplate* tmpl,
                          const GenreProfile& profile) const;

    // Resolve a role to a semitone offset from root (not APPROACH/ANY)
    int resolveRoleToSemitone(PitchRole role,
                               int preferredIndex,
                               const juce::Array<PitchRole>& preferred) const;

    // Pick a semitone from preferredIntervals avoiding recentSemitone
    int pickFromPreferred(const juce::Array<PitchRole>& preferred,
                          int& preferredIndex,
                          int recentSemitone) const;

    // Clamp a MIDI note to the genre's bass range, shifting by octave if needed
    int clampToRange(int midiNote, int rootMidi, int rangeMin, int rangeMax) const;
};
