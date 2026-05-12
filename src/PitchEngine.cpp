#include "PitchEngine.h"

void PitchEngine::reset()
{
    std::fill(barNotes, barNotes + 16, -1);
    ready = false;
}

int PitchEngine::getNoteForStep(int step) const
{
    if (!ready || step < 0 || step >= 16) return -1;
    return barNotes[step];
}

PitchRole PitchEngine::roleForStep(int step,
                                    const GrooveTemplate* tmpl,
                                    const GenreProfile& profile,
                                    int bar) const
{
    juce::ignoreUnused(profile);
    // 1. Explicit step hint in template takes priority
    if (tmpl && tmpl->pitch.hasPitchData)
    {
        for (auto& h : tmpl->pitch.stepHints)
            if (h.step == step) return h.role;
    }

    // 2. Lock point default (bar-aware)
    if (tmpl)
    {
        auto lp = tmpl->lockAt(step, bar);
        if (lp)
        {
            switch (lp->type)
            {
                case LockType::UNISON:
                    return PitchRole::ROOT;
                case LockType::ALTERNATE:
                    return (step % 4 == 0) ? PitchRole::FIFTH : PitchRole::FLAT7;
                case LockType::ANTICIPATE:
                    return PitchRole::APPROACH;
                case LockType::FILL:
                    return PitchRole::ANY;
            }
        }

        // 3. Inherit from nearest lock point within 2 steps
        const auto& lockSource = (bar == 1 && !tmpl->locks2.isEmpty()) ? tmpl->locks2 : tmpl->locks;
        for (int dist = 1; dist <= 2; ++dist)
        {
            int prev = (step - dist + 16) % 16;
            int next = (step + dist) % 16;
            for (auto& lk : lockSource)
            {
                if (lk.step == prev || lk.step == next)
                {
                    switch (lk.type)
                    {
                        case LockType::UNISON:     return PitchRole::ROOT;
                        case LockType::ALTERNATE:  return PitchRole::FIFTH;
                        case LockType::ANTICIPATE: return PitchRole::APPROACH;
                        case LockType::FILL:       return PitchRole::ANY;
                    }
                }
            }
        }
    }

    return PitchRole::ROOT;
}

int PitchEngine::resolveRoleToSemitone(PitchRole role,
                                        int preferredIndex,
                                        const juce::Array<PitchRole>& preferred) const
{
    if (role == PitchRole::ANY || role == PitchRole::NONE)
    {
        // Use the preferred interval at the given index, defaulting to root
        if (!preferred.isEmpty())
        {
            int idx = preferredIndex % preferred.size();
            PitchRole r = preferred[idx];
            if (r != PitchRole::ANY && r != PitchRole::APPROACH && r != PitchRole::NONE)
                return semitoneForRole(r);
        }
        return 0;
    }
    return semitoneForRole(role);
}

int PitchEngine::pickFromPreferred(const juce::Array<PitchRole>& preferred,
                                    int& preferredIndex,
                                    int recentSemitone) const
{
    if (preferred.isEmpty()) return 0;

    // Try each preferred interval starting from preferredIndex, skip if same as recent
    int n = preferred.size();
    for (int attempt = 0; attempt < n; ++attempt)
    {
        int idx = (preferredIndex + attempt) % n;
        PitchRole r = preferred[idx];
        if (r == PitchRole::ANY || r == PitchRole::APPROACH) continue;
        int semi = semitoneForRole(r);
        if (semi != recentSemitone || n == 1)
        {
            preferredIndex = (idx + 1) % n;
            return semi;
        }
    }
    preferredIndex = (preferredIndex + 1) % n;
    return semitoneForRole(preferred[0]);
}

int PitchEngine::clampToRange(int midiNote, int rootMidi, int rangeMin, int rangeMax) const
{
    juce::ignoreUnused(rootMidi);
    while (midiNote > rangeMax) midiNote -= 12;
    while (midiNote < rangeMin) midiNote += 12;
    return juce::jlimit(0, 127, midiNote);
}

void PitchEngine::computeBar(const GrooveTemplate* tmpl,
                              const GenreProfile& profile,
                              const PitchEngineParams& params)
{
    std::fill(barNotes, barNotes + 16, -1);
    ready = true;

    if (!params.pitchEnabled)
    {
        // All active steps → root note
        std::fill(barNotes, barNotes + 16, params.rootMidiNote);
        return;
    }

    // Determine effective settings from template pitch block or genre profile
    int density = params.densityOverride > 0 ? params.densityOverride
                : (tmpl && tmpl->pitch.hasPitchData ? tmpl->pitch.densityHint
                                                    : profile.pitchDensityMin + 1);
    density = juce::jlimit(1, 5, density);

    bool chromatic = params.chromaticApproach;
    if (tmpl && tmpl->pitch.hasPitchData)
        chromatic = tmpl->pitch.allowChromaticApproach && chromatic;
    else
        chromatic = profile.allowChromaticApproach && chromatic;

    const auto& preferred = (tmpl && tmpl->pitch.hasPitchData && !tmpl->pitch.preferredIntervals.isEmpty())
                           ? tmpl->pitch.preferredIntervals
                           : profile.preferredIntervals;

    // ── Pass 1: resolve all non-APPROACH steps ──────────────────────────────
    int uniqueNotes[10]; int uniqueCount = 0;
    int recentSemitone = -1;
    int preferredIdx   = 0;

    const auto& activeBass = (params.barVariant == 1 && tmpl && !tmpl->bass2.isEmpty())
                           ? tmpl->bass2 : tmpl->bass;

    for (int step = 0; step < 16; ++step)
    {
        bool active = tmpl && !activeBass.isEmpty() && activeBass[0]->steps[step] > 0;
        if (!active) { barNotes[step] = params.rootMidiNote; continue; }

        PitchRole role = roleForStep(step, tmpl, profile, params.barVariant);

        if (role == PitchRole::APPROACH)
        {
            barNotes[step] = -2; // sentinel — resolve in pass 2
            continue;
        }

        int semi;
        if (role == PitchRole::ANY)
            semi = pickFromPreferred(preferred, preferredIdx, recentSemitone);
        else
            semi = resolveRoleToSemitone(role, preferredIdx, preferred);

        if (role != PitchRole::FLAT5 && role != PitchRole::OCTAVE)
            semi = snapToScale(semi, params.scaleType);

        // Compute MIDI note first so octave variants are distinct entries
        int midiNote = clampToRange(params.rootMidiNote + semi,
                                    params.rootMidiNote, profile.bassMidiMin, profile.bassMidiMax);

        // Density limiting: track unique MIDI notes (not pitch classes)
        if (role != PitchRole::ROOT)
        {
            bool alreadyUsed = false;
            for (int i = 0; i < uniqueCount; ++i)
                if (uniqueNotes[i] == midiNote) { alreadyUsed = true; break; }
            if (!alreadyUsed && uniqueCount >= density)
                midiNote = params.rootMidiNote; // fall back to root MIDI note
        }

        bool tracked = false;
        for (int i = 0; i < uniqueCount; ++i)
            if (uniqueNotes[i] == midiNote) { tracked = true; break; }
        if (!tracked && uniqueCount < 10)
            uniqueNotes[uniqueCount++] = midiNote;

        barNotes[step] = midiNote;
        recentSemitone = semi;
    }

    // ── Pass 2: resolve APPROACH steps (need next active step's note) ───────
    for (int step = 0; step < 16; ++step)
    {
        if (barNotes[step] != -2) continue;

        // Find next active step's resolved note
        int targetNote = params.rootMidiNote;
        for (int d = 1; d <= 16; ++d)
        {
            int next = (step + d) % 16;
            if (barNotes[next] >= 0)
            {
                targetNote = barNotes[next];
                break;
            }
        }

        int approachNote;
        if (chromatic)
        {
            approachNote = targetNote - 1; // chromatic half-step below
        }
        else
        {
            // Nearest scale tone below
            int targetSemi = (targetNote - params.rootMidiNote) % 12;
            int count = 0;
            const int* ints = scaleIntervals(params.scaleType, count);
            int belowSemi = 0;
            for (int i = 0; i < count; ++i)
                if (ints[i] < targetSemi) belowSemi = ints[i];
            approachNote = params.rootMidiNote + belowSemi;
        }

        approachNote = clampToRange(approachNote, params.rootMidiNote,
                                    profile.bassMidiMin, profile.bassMidiMax);
        barNotes[step] = approachNote;
    }

    // Replace any remaining -1 sentinel (shouldn't happen, safety net)
    for (int step = 0; step < 16; ++step)
        if (barNotes[step] < 0) barNotes[step] = params.rootMidiNote;
}

void PitchEngine::computeBarFromState(const BarPitchState&     state,
                                       const GenreProfile&      profile,
                                       const PitchEngineParams& params)
{
    std::fill(barNotes, barNotes + 16, -1);
    ready = true;

    if (!params.pitchEnabled)
    {
        std::fill(barNotes, barNotes + 16, params.rootMidiNote);
        return;
    }

    int density = params.densityOverride > 0 ? params.densityOverride
                : profile.pitchDensityMin + 1;
    density = juce::jlimit(1, 5, density);

    bool chromatic = params.chromaticApproach && profile.allowChromaticApproach;

    const auto& preferred = profile.preferredIntervals;

    int uniqueNotes[10]; int uniqueCount = 0;
    int recentSemitone = -1;
    int preferredIdx   = 0;

    // ── Pass 1: resolve non-approach steps ──────────────────────────────────
    for (int step = 0; step < 16; ++step)
    {
        PitchRole role = state.stepRoles[step];

        if (role == PitchRole::NONE)
        {
            barNotes[step] = params.rootMidiNote;
            continue;
        }
        if (role == PitchRole::APPROACH)
        {
            barNotes[step] = -2; // resolve in pass 2
            continue;
        }

        int semi;
        if (role == PitchRole::ANY)
            semi = pickFromPreferred(preferred, preferredIdx, recentSemitone);
        else
            semi = resolveRoleToSemitone(role, preferredIdx, preferred);

        if (role != PitchRole::FLAT5 && role != PitchRole::OCTAVE)
            semi = snapToScale(semi, params.scaleType);

        // Compute MIDI note first (including phrase octave displacement) so
        // octave variants are tracked as distinct entries
        int midiNote = clampToRange(params.rootMidiNote + semi + state.stepOctaveOffset[step] * 12,
                                    params.rootMidiNote, profile.bassMidiMin, profile.bassMidiMax);

        if (role != PitchRole::ROOT)
        {
            bool alreadyUsed = false;
            for (int i = 0; i < uniqueCount; ++i)
                if (uniqueNotes[i] == midiNote) { alreadyUsed = true; break; }
            if (!alreadyUsed && uniqueCount >= density)
                midiNote = params.rootMidiNote;
        }

        bool tracked = false;
        for (int i = 0; i < uniqueCount; ++i)
            if (uniqueNotes[i] == midiNote) { tracked = true; break; }
        if (!tracked && uniqueCount < 10)
            uniqueNotes[uniqueCount++] = midiNote;

        barNotes[step] = midiNote;
        recentSemitone = semi;
    }

    // ── Pass 2: resolve approach steps ──────────────────────────────────────
    for (int step = 0; step < 16; ++step)
    {
        if (barNotes[step] != -2) continue;

        int targetNote = params.rootMidiNote;
        for (int d = 1; d <= 16; ++d)
        {
            int next = (step + d) % 16;
            if (barNotes[next] >= 0) { targetNote = barNotes[next]; break; }
        }

        int approachNote;
        if (chromatic)
        {
            approachNote = targetNote - 1;
        }
        else
        {
            int targetSemi = (targetNote - params.rootMidiNote) % 12;
            int count = 0;
            const int* ints = scaleIntervals(params.scaleType, count);
            int belowSemi = 0;
            for (int i = 0; i < count; ++i)
                if (ints[i] < targetSemi) belowSemi = ints[i];
            approachNote = params.rootMidiNote + belowSemi;
        }

        approachNote = clampToRange(approachNote, params.rootMidiNote,
                                    profile.bassMidiMin, profile.bassMidiMax);
        barNotes[step] = approachNote;
    }

    for (int step = 0; step < 16; ++step)
        if (barNotes[step] < 0) barNotes[step] = params.rootMidiNote;
}
