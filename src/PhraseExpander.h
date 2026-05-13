#pragma once
#include <JuceHeader.h>
#include "PitchTypes.h"
#include "GrooveTemplate.h"
#include "GenreProfile.h"

class PhraseExpander
{
public:
    // Compute 8-bar pitch phrase from seed template + user control values.
    // density: 0.0 (sparse/sparse) — 1.0 (busy/active)
    // tension: 0.0 (safe/consonant) — 1.0 (adventurous/chromatic)
    // randomSeed: 0 = vary each call; non-zero = deterministic result
    void compute(const GrooveTemplate* tmpl,
                 const GenreProfile&   profile,
                 float density, float tension,
                 juce::int64 randomSeed = 0);

    const ExpandedPhrase& getPhrase() const { return phrase; }

private:
    ExpandedPhrase phrase;
    juce::Random   rng;

    // Mirrors PitchEngine::roleForStep — seed role before any phrase transformation
    // bar=0 → locks (bar 1), bar=1 → locks2 (bar 2, falls back to locks when empty)
    PitchRole seedRoleForStep(int step, const GrooveTemplate* tmpl,
                               const GenreProfile& profile, int bar = 0) const;

    bool stepIsActive (int step, const GrooveTemplate* tmpl, int bar = 0) const;
    bool isUnisonStep (int step, const GrooveTemplate* tmpl, int bar = 0) const;

    // Build the subset of preferredIntervals accessible at this tension level
    void buildAvailableRoles(float tension, const GenreProfile& profile,
                              juce::Array<PitchRole>& out) const;

    // Assign pitch roles for one bar using deviation + tension
    void computeBarRoles(int barIdx, float effectiveDev, float tension,
                         const GrooveTemplate* tmpl,
                         const juce::Array<PitchRole>& availRoles,
                         const PitchRole* seedRoles,
                         BarPitchState& out, int bar = 0);

    // Enforce minimum root-note fraction per bar (Part 3, Rule 1)
    void enforceRootGravity(int barIdx, const GrooveTemplate* tmpl,
                             BarPitchState& out, int bar = 0);

    // Enforce that non-root fills resolve back to root within N active steps (Hyphy)
    void enforceFillResolution(BarPitchState& out, const GenreProfile& profile) const;

    // b7 leading-tone turnaround on last active step of bars 3 and 7 (Part 3, Rule 6)
    void applyTurnaround(int barIdx, float tension, const GrooveTemplate* tmpl,
                          BarPitchState& out, int bar = 0) const;

    // Octave displacement per bar based on deviation × density (Part 1, Rule 3)
    void computeOctaveOffsets(int barIdx, float effectiveDev, float density,
                               const GrooveTemplate* tmpl, const GenreProfile& profile,
                               BarPitchState& out, int bar = 0);
};
