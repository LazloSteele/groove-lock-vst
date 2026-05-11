#include "PhraseExpander.h"

// ─── Public ───────────────────────────────────────────────────────────────────

void PhraseExpander::compute(const GrooveTemplate* tmpl,
                              const GenreProfile&   profile,
                              float density, float tension,
                              juce::int64 randomSeed)
{
    if (randomSeed != 0)
        rng = juce::Random(randomSeed);

    density = juce::jlimit(profile.densityClampMin, profile.densityClampMax, density);
    tension = juce::jlimit(profile.tensionClampMin, profile.tensionClampMax, tension);

    PitchRole seedRoles[16];
    for (int s = 0; s < 16; ++s)
        seedRoles[s] = seedRoleForStep(s, tmpl, profile);

    juce::Array<PitchRole> availRoles;
    buildAvailableRoles(tension, profile, availRoles);

    for (int b = 0; b < 8; ++b)
    {
        float effectiveDev = juce::jlimit(0.f, 1.f,
                                          phrase.phraseArc[b] * profile.maxDeviation * density);
        phrase.bars[b].deviationLevel = effectiveDev;

        computeBarRoles   (b, effectiveDev, tension, tmpl, availRoles, seedRoles, phrase.bars[b], 0);
        enforceRootGravity(b, tmpl, phrase.bars[b], 0);
        applyTurnaround   (b, tension, tmpl, phrase.bars[b], 0);
        computeOctaveOffsets(b, effectiveDev, density, tmpl, profile, phrase.bars[b], 0);
    }

    // Compute bar-2 phrase if template has bass2
    phrase.hasBar2 = false;
    if (tmpl && !tmpl->bass2.isEmpty())
    {
        PitchRole seedRoles2[16];
        for (int s = 0; s < 16; ++s)
            seedRoles2[s] = seedRoleForStep(s, tmpl, profile, 1);

        for (int b = 0; b < 8; ++b)
        {
            float effectiveDev = juce::jlimit(0.f, 1.f,
                                              phrase.phraseArc[b] * profile.maxDeviation * density);
            phrase.bars2[b].deviationLevel = effectiveDev;

            computeBarRoles   (b, effectiveDev, tension, tmpl, availRoles, seedRoles2, phrase.bars2[b], 1);
            enforceRootGravity(b, tmpl, phrase.bars2[b], 1);
            applyTurnaround   (b, tension, tmpl, phrase.bars2[b], 1);
            computeOctaveOffsets(b, effectiveDev, density, tmpl, profile, phrase.bars2[b], 1);
        }
        phrase.hasBar2 = true;
    }

    phrase.isValid = true;
}

// ─── Private helpers ──────────────────────────────────────────────────────────

PitchRole PhraseExpander::seedRoleForStep(int step, const GrooveTemplate* tmpl,
                                           const GenreProfile& profile, int bar) const
{
    juce::ignoreUnused(profile);
    if (!tmpl) return PitchRole::ROOT;

    if (tmpl->pitch.hasPitchData)
        for (auto& h : tmpl->pitch.stepHints)
            if (h.step == step) return h.role;

    auto lp = tmpl->lockAt(step, bar);
    if (lp)
    {
        switch (lp->type)
        {
            case LockType::UNISON:     return PitchRole::ROOT;
            case LockType::ALTERNATE:  return (step % 4 == 0) ? PitchRole::FIFTH : PitchRole::FLAT7;
            case LockType::ANTICIPATE: return PitchRole::APPROACH;
            case LockType::FILL:       return PitchRole::ANY;
        }
    }

    const auto& lockSource = (bar == 1 && !tmpl->locks2.isEmpty()) ? tmpl->locks2 : tmpl->locks;
    for (int dist = 1; dist <= 2; ++dist)
    {
        for (auto& lk : lockSource)
        {
            if (lk.step == (step - dist + 16) % 16 || lk.step == (step + dist) % 16)
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

    return PitchRole::ROOT;
}

bool PhraseExpander::stepIsActive(int step, const GrooveTemplate* tmpl, int bar) const
{
    if (!tmpl) return false;
    const auto& src = (bar == 1 && !tmpl->bass2.isEmpty()) ? tmpl->bass2 : tmpl->bass;
    return !src.isEmpty() && src[0]->steps[step] > 0;
}

bool PhraseExpander::isUnisonStep(int step, const GrooveTemplate* tmpl, int bar) const
{
    if (!tmpl) return false;
    auto lp = tmpl->lockAt(step, bar);
    return lp.has_value() && lp->type == LockType::UNISON;
}

void PhraseExpander::buildAvailableRoles(float tension, const GenreProfile& profile,
                                          juce::Array<PitchRole>& out) const
{
    out.clear();
    const auto& pref = profile.preferredIntervals;

    // Tension controls how far into preferredIntervals we can reach
    int maxIdx;
    if      (tension < 0.2f) maxIdx = 2;
    else if (tension < 0.4f) maxIdx = 3;
    else if (tension < 0.6f) maxIdx = 4;
    else                     maxIdx = pref.size();
    maxIdx = juce::jmin(maxIdx, pref.size());

    for (int i = 0; i < maxIdx; ++i)
        out.addIfNotAlreadyThere(pref[i]);

    // ROOT and FIFTH are always accessible regardless of tension or ordering
    out.addIfNotAlreadyThere(PitchRole::ROOT);
    out.addIfNotAlreadyThere(PitchRole::FIFTH);

    // At high tension the b5 blue note becomes available (adds harmonic edge)
    if (tension >= 0.8f && profile.allowChromaticApproach)
        out.addIfNotAlreadyThere(PitchRole::FLAT5);
}

void PhraseExpander::computeBarRoles(int barIdx, float effectiveDev, float tension,
                                      const GrooveTemplate* tmpl,
                                      const juce::Array<PitchRole>& availRoles,
                                      const PitchRole* seedRoles,
                                      BarPitchState& out, int bar)
{
    float actualDev = (barIdx == 7) ? juce::jmin(effectiveDev, 0.15f) : effectiveDev;

    for (int step = 0; step < 16; ++step)
    {
        if (!stepIsActive(step, tmpl, bar))
        {
            out.stepRoles[step] = PitchRole::NONE;
            continue;
        }

        PitchRole seed = seedRoles[step];

        // Root (unison lock points) and approach notes are immutable
        if (seed == PitchRole::ROOT || seed == PitchRole::APPROACH || seed == PitchRole::NONE)
        {
            out.stepRoles[step] = seed;
            continue;
        }

        // Probability that this step's role rotates to something different.
        // Baseline ensures tension is always audible even in early bars.
        float rotChance = juce::jlimit(0.f, 0.85f,
                                       actualDev * tension + tension * 0.25f);

        if (rng.nextFloat() < rotChance && availRoles.size() > 2)
        {
            juce::Array<PitchRole> candidates;
            for (auto r : availRoles)
                if (r != PitchRole::ROOT && r != PitchRole::APPROACH &&
                    r != PitchRole::NONE  && r != seed)
                    candidates.add(r);

            out.stepRoles[step] = candidates.isEmpty()
                                    ? seed
                                    : candidates[rng.nextInt(candidates.size())];
        }
        else
        {
            // Keep seed role, but clamp to what tension allows
            if (availRoles.contains(seed) || seed == PitchRole::ANY)
                out.stepRoles[step] = seed;
            else
            {
                // Seed role is above current tension ceiling — use nearest available
                PitchRole fallback = PitchRole::ROOT;
                for (auto r : availRoles)
                    if (r != PitchRole::ROOT && r != PitchRole::APPROACH)
                    { fallback = r; break; }
                out.stepRoles[step] = fallback;
            }
        }
    }
}

void PhraseExpander::enforceRootGravity(int barIdx, const GrooveTemplate* tmpl,
                                         BarPitchState& out, int bar)
{
    // Minimum root fraction per bar (Part 3, Rule 1)
    static const float ROOT_FLOORS[8] = {0.60f, 0.60f, 0.40f, 0.40f, 0.30f, 0.30f, 0.25f, 0.70f};
    float floor = ROOT_FLOORS[barIdx];

    int activeCount = 0, rootCount = 0;
    for (int s = 0; s < 16; ++s)
    {
        if (out.stepRoles[s] == PitchRole::NONE) continue;
        ++activeCount;
        if (out.stepRoles[s] == PitchRole::ROOT) ++rootCount;
    }
    if (activeCount == 0) return;

    float rootFrac = (float)rootCount / (float)activeCount;
    if (rootFrac >= floor) return;

    int needed = (int)std::ceil(floor * activeCount) - rootCount;

    // First pass: convert ANY/FILL steps (most expendable)
    for (int s = 0; s < 16 && needed > 0; ++s)
    {
        if (out.stepRoles[s] == PitchRole::NONE  ||
            out.stepRoles[s] == PitchRole::ROOT   ||
            out.stepRoles[s] == PitchRole::APPROACH) continue;
        if (isUnisonStep(s, tmpl, bar)) continue;
        if (out.stepRoles[s] == PitchRole::ANY)
        { out.stepRoles[s] = PitchRole::ROOT; --needed; }
    }
    for (int s = 0; s < 16 && needed > 0; ++s)
    {
        if (out.stepRoles[s] == PitchRole::NONE  ||
            out.stepRoles[s] == PitchRole::ROOT   ||
            out.stepRoles[s] == PitchRole::APPROACH) continue;
        out.stepRoles[s] = PitchRole::ROOT;
        --needed;
    }
}

void PhraseExpander::applyTurnaround(int barIdx, float tension, const GrooveTemplate* tmpl,
                                      BarPitchState& out, int bar) const
{
    if (barIdx != 3 && barIdx != 7) return;

    int lastActive = -1;
    for (int s = 15; s >= 0; --s)
        if (out.stepRoles[s] != PitchRole::NONE) { lastActive = s; break; }
    if (lastActive < 0) return;

    if (isUnisonStep(lastActive, tmpl, bar)) return;

    // tension > 0.2: use b7 as leading tone. At very low tension keep root (consonant loop).
    out.stepRoles[lastActive] = (tension > 0.2f) ? PitchRole::FLAT7 : PitchRole::ROOT;
}

void PhraseExpander::computeOctaveOffsets(int barIdx, float effectiveDev, float density,
                                           const GrooveTemplate* tmpl,
                                           const GenreProfile& profile,
                                           BarPitchState& out, int bar)
{
    std::fill(out.stepOctaveOffset, out.stepOctaveOffset + 16, 0);
    if (profile.maxOctaveDisplPerBar == 0) return;

    static const float OCT_CONTOUR[8] = {0.0f, 0.0f, 0.10f, 0.20f, 0.35f, 0.50f, 0.75f, 0.0f};
    float baseProb = OCT_CONTOUR[barIdx] * density;
    if (baseProb < 0.05f) return;

    const auto& activeBass = (bar == 1 && tmpl && !tmpl->bass2.isEmpty()) ? tmpl->bass2 : tmpl->bass;

    int displCount = 0;
    for (int s = 0; s < 16; ++s)
    {
        if (displCount >= profile.maxOctaveDisplPerBar) break;
        if (out.stepRoles[s] == PitchRole::NONE    ||
            out.stepRoles[s] == PitchRole::ROOT     ||
            out.stepRoles[s] == PitchRole::APPROACH) continue;
        if (isUnisonStep(s, tmpl, bar)) continue;

        if (tmpl && !activeBass.isEmpty() && activeBass[0]->steps[s] <= 1) continue;

        float prob = juce::jmin(1.f, baseProb * (1.f + effectiveDev * 0.5f));
        if (rng.nextFloat() < prob)
        {
            out.stepOctaveOffset[s] = 1;
            ++displCount;
        }
    }
}
