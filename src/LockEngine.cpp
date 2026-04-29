#include "LockEngine.h"

LockEngine::LockEngine() { reset(); }

void LockEngine::reset()
{
    lastStep         = -1;
    lastNoteOnSample = -1;
    pitchEngine.reset();
}

void LockEngine::setTemplate(const GrooveTemplate* t)
{
    tmpl    = t;
    profile = t ? GenreProfile::forGenre(t->genre) : GenreProfile::forGenre("Modern West Coast");
    reset();
}

void LockEngine::setParams(const LockEngineParams& p) { params = p; }

// ─── Helpers ──────────────────────────────────────────────────────────────────

float LockEngine::velForTier(int tier) const
{
    auto rnd = [&](int lo, int hi) -> float {
        return (float)(lo + random.nextInt(hi - lo + 1));
    };
    switch (tier)
    {
        case 1: return rnd(profile.ghostVelMin,  profile.ghostVelMax);
        case 2: return rnd(profile.medVelMin,    profile.medVelMax);
        case 3: return rnd(profile.fullVelMin,   profile.fullVelMax);
        case 4: return rnd(profile.accentVelMin, profile.accentVelMax);
        default: return 0.f;
    }
}

float LockEngine::timingOffsetSamples(BassArt art, double sampleRate) const
{
    float ms = profile.defaultTimingOffsetMs + params.timingOffsetMs;
    switch (art)
    {
        case BassArt::PUSH: ms += profile.pushOffsetMs; break;
        case BassArt::LAY:  ms += profile.layOffsetMs;  break;
        default: break;
    }
    float humanAmt = (params.humanizePercent / 100.f) * (1.f - profile.timingToVelocityRatio);
    float jitter   = (random.nextFloat() * 2.f - 1.f) * profile.timingJitterMaxMs * humanAmt;
    ms += jitter;
    return (float)(ms * sampleRate / 1000.0);
}

float LockEngine::gatePercent(BassArt art) const
{
    float g = profile.defaultGatePercent;
    if (art == BassArt::STACCATO) g = profile.staccatoGatePercent;
    if (art == BassArt::LEGATO)   g = profile.legatoGatePercent;
    float humanAmt = (params.humanizePercent / 100.f);
    float jitter   = (random.nextFloat() * 2.f - 1.f) * profile.gateHumanizeRange * humanAmt;
    return juce::jlimit(0.15f, 1.0f, g * params.gateLengthScale + jitter);
}

// ─── Adaptive step movement ───────────────────────────────────────────────────

AdaptedPattern LockEngine::computeAdaptation(const DrumState& drums,
                                              const GrooveTemplate* tmpl)
{
    AdaptedPattern result;
    if (!tmpl || tmpl->bass.isEmpty()) return result;

    auto* bassRow = tmpl->bass[0];

    // No adaptation if drummer isn't playing kicks
    bool anyKick = false;
    for (int i = 0; i < 16; ++i) if (drums.kickHits[i] > 0) { anyKick = true; break; }
    if (!anyKick) return result; // valid=false → seed plays unmodified

    int kickMap[16], snareMap[16];
    for (int i = 0; i < 16; ++i) {
        kickMap[i]  = drums.kickHits[i];
        snareMap[i] = drums.snareHits[i];
    }

    // resolvedPos[seedStep] = output step (0-15), or -1 if muted
    int  resolvedPos[16];
    bool occupied[16];
    std::fill(resolvedPos, resolvedPos + 16, -1);
    std::fill(occupied,    occupied    + 16, false);

    // Helper: lock type for a seed step (returns {hasLock, type})
    auto lockOf = [&](int s) -> std::pair<bool, LockType> {
        auto lp = tmpl->lockAt(s);
        if (!lp) return {false, LockType::FILL};
        return {true, lp->type};
    };

    // PHASE 1: UNISON — snap to nearest kick within ±kUnisonMaxMove
    // UNISON can cross beat boundaries.
    for (int s = 0; s < 16; ++s)
    {
        if (bassRow->steps[s] == 0) continue;
        auto [has, lt] = lockOf(s);
        if (!has || lt != LockType::UNISON) continue;

        int best = -1;
        for (int d = 0; d <= kUnisonMaxMove && best < 0; ++d)
        {
            if (d == 0) {
                if (kickMap[s] > 0 && !occupied[s]) best = s;
            } else {
                int tl = s - d, tr = s + d;
                if (tl >= 0  && kickMap[tl] > 0 && !occupied[tl]) best = tl;
                if (best < 0 && tr < 16 && kickMap[tr] > 0 && !occupied[tr]) best = tr;
            }
        }

        if (best >= 0) { resolvedPos[s] = best; occupied[best] = true; }
        // else muted (resolvedPos[s] stays -1)
    }

    // PHASE 2: ANTICIPATE — land one step before the next resolved unison position.
    // Does NOT cross beat boundaries.
    for (int s = 0; s < 16; ++s)
    {
        if (bassRow->steps[s] == 0) continue;
        auto [has, lt] = lockOf(s);
        if (!has || lt != LockType::ANTICIPATE) continue;

        // Find next unison seed step's resolved output position
        int nextUnisonOut = -1;
        for (int i = 1; i <= 16; ++i)
        {
            int ns = (s + i) & 15;
            if (bassRow->steps[ns] == 0) continue;
            auto [has2, lt2] = lockOf(ns);
            if (has2 && lt2 == LockType::UNISON) { nextUnisonOut = resolvedPos[ns]; break; }
        }

        int beatStart = (s / 4) * 4;
        int beatEnd   = beatStart + 3;

        // Target is one before the unison; fall back to seed if unresolved or at beat edge
        int targetPos = (nextUnisonOut > 0) ? nextUnisonOut - 1 : s;
        if (targetPos < beatStart || targetPos > beatEnd) targetPos = s; // no cross-beat

        if (!occupied[targetPos]) {
            resolvedPos[s] = targetPos; occupied[targetPos] = true;
        } else if (targetPos != s && !occupied[s]) {
            resolvedPos[s] = s; occupied[s] = true;
        }
        // else muted
    }

    // PHASE 3: ALTERNATE — move to emptiest gap in the same beat within ±kAlternateMaxMove.
    // Does NOT cross beat boundaries.
    for (int s = 0; s < 16; ++s)
    {
        if (bassRow->steps[s] == 0) continue;
        auto [has, lt] = lockOf(s);
        if (!has || lt != LockType::ALTERNATE) continue;

        int beatStart = (s / 4) * 4;
        int beatEnd   = beatStart + 3;

        int best = -1;
        for (int d = 0; d <= kAlternateMaxMove && best < 0; ++d)
        {
            if (d == 0) {
                if (!occupied[s] && kickMap[s] == 0 && snareMap[s] == 0) best = s;
            } else {
                int tl = s - d, tr = s + d;
                if (tl >= beatStart && !occupied[tl] && kickMap[tl] == 0 && snareMap[tl] == 0)
                    best = tl;
                if (best < 0 && tr <= beatEnd && !occupied[tr] && kickMap[tr] == 0 && snareMap[tr] == 0)
                    best = tr;
            }
        }

        if (best >= 0) { resolvedPos[s] = best; occupied[best] = true; }
        else if (!occupied[s] && kickMap[s] == 0 && snareMap[s] == 0) {
            resolvedPos[s] = s; occupied[s] = true; // keep original if safe
        }
        // else muted (drum is on original position, no gap found)
    }

    // PHASE 4: FILL and no-lock-point — never move; mute if drum present.
    for (int s = 0; s < 16; ++s)
    {
        if (bassRow->steps[s] == 0) continue;
        auto [has, lt] = lockOf(s);
        if (has && lt != LockType::FILL) continue; // already processed

        if (!occupied[s] && kickMap[s] == 0 && snareMap[s] == 0) {
            resolvedPos[s] = s; occupied[s] = true;
        }
        // else muted (drum competes with fill position)
    }

    // PHASE 5a: Contour check — cancel movement if adapted pair is too close
    // but was well-spaced in the seed. Lower priority note's movement is cancelled.
    // Priority (highest to lowest): UNISON > ANTICIPATE > ALTERNATE > FILL/none
    auto lockPriority = [&](int s) -> int {
        auto [has, lt] = lockOf(s);
        if (!has || lt == LockType::FILL) return 0;
        if (lt == LockType::ALTERNATE)   return 1;
        if (lt == LockType::ANTICIPATE)  return 2;
        if (lt == LockType::UNISON)      return 3;
        return 0;
    };

    for (int i = 0; i < 16; ++i)
    {
        if (!occupied[i]) continue;
        // Find next occupied output step
        int j = -1;
        for (int k = i + 1; k < 16; ++k) { if (occupied[k]) { j = k; break; } }
        if (j < 0) break;

        int outDist = j - i;
        if (outDist >= kMinNoteSpacing) continue;

        // Find which seed steps map to i and j
        int seedI = -1, seedJ = -1;
        for (int s = 0; s < 16; ++s) {
            if (resolvedPos[s] == i) seedI = s;
            if (resolvedPos[s] == j) seedJ = s;
        }
        if (seedI < 0 || seedJ < 0) continue;

        int seedDist = std::abs(seedJ - seedI);
        if (seedDist < 3) continue; // was already close in seed, no correction needed

        // Cancel the lower-priority note's movement (return it to seed position if free)
        int prioI = lockPriority(seedI), prioJ = lockPriority(seedJ);
        int lowerSeed = (prioI <= prioJ) ? seedI : seedJ;
        int lowerOut  = resolvedPos[lowerSeed];

        occupied[lowerOut] = false;
        resolvedPos[lowerSeed] = -1;

        if (!occupied[lowerSeed]) {
            resolvedPos[lowerSeed] = lowerSeed;
            occupied[lowerSeed] = true;
        }
        // else: stays muted
    }

    // PHASE 5b: Beat vacated check — if a beat had a seed note but has none after
    // adaptation, insert a ghost fill at the vacated seed position.
    bool ghostFill[16] = {};
    for (int beat = 0; beat < 4; ++beat)
    {
        int bs = beat * 4, be = bs + 3;
        int vacatedSeedPos = -1;
        bool beatOccupiedInSeed = false, beatOccupiedInAdapted = false;

        for (int s = bs; s <= be; ++s)
            if (bassRow->steps[s] > 0) { beatOccupiedInSeed = true; vacatedSeedPos = s; }
        for (int s = bs; s <= be; ++s)
            if (occupied[s]) { beatOccupiedInAdapted = true; break; }

        if (beatOccupiedInSeed && !beatOccupiedInAdapted && vacatedSeedPos >= 0)
        {
            if (!occupied[vacatedSeedPos]) {
                resolvedPos[vacatedSeedPos] = vacatedSeedPos;
                occupied[vacatedSeedPos]    = true;
                ghostFill[vacatedSeedPos]   = true;
            }
        }
    }

    // PHASE 6: Pitch role assignment
    // For each resolved note, determine pitch role based on output position.
    auto pitchRoleForOutput = [&](int seedPos, int outPos) -> PitchRole {
        // Landed on a kick → root (unison behavior regardless of original lock type)
        if (kickMap[outPos] > 0) return PitchRole::ROOT;
        // One step before a kick → approach
        int nextStep = (outPos + 1) & 15;
        if (kickMap[nextStep] > 0) return PitchRole::APPROACH;
        // Use the seed's lock type to determine the original intended role
        auto [has, lt] = lockOf(seedPos);
        if (!has || lt == LockType::FILL)       return PitchRole::ANY;
        if (lt == LockType::UNISON)             return PitchRole::ROOT;
        if (lt == LockType::ALTERNATE)          return PitchRole::FIFTH;
        if (lt == LockType::ANTICIPATE)         return PitchRole::APPROACH;
        return PitchRole::ANY;
    };

    // Build the final AdaptedPattern
    for (int s = 0; s < 16; ++s)
    {
        int outPos = resolvedPos[s];
        if (outPos < 0 || bassRow->steps[s] == 0) continue;

        AdaptedStep& as = result.steps[outPos];
        as.seedPosition    = s;
        as.velocityTier    = ghostFill[s] ? 1 : bassRow->steps[s];
        as.pitchRole       = pitchRoleForOutput(s, outPos);
        as.muted           = false;
        as.ghostFill       = ghostFill[s];
        as.kickVelTracking = (kickMap[outPos] > 0) ? (float)kickMap[outPos] : 0.f;

        // Articulation: use seed's; if note moved AND is an anticipate, force slide
        BassArt art = bassRow->timing[s];
        {
            auto [has, lt] = lockOf(s);
            if (has && lt == LockType::ANTICIPATE && outPos != s)
                art = BassArt::SLIDE;
            if (has && lt == LockType::FILL)
                art = BassArt::STACCATO; // fills always staccato
            if (ghostFill[s])
                art = BassArt::STACCATO;
        }
        as.articulation = art;
        as.gateArt      = (tmpl->bass.size() > 1) ? tmpl->bass[1]->timing[s] : art;
        if (ghostFill[s]) as.gateArt = BassArt::STACCATO;
    }

    result.valid = true;
    return result;
}

// ─── processStep ──────────────────────────────────────────────────────────────

void LockEngine::processStep(int step, int64 stepSamplePos, double sampleRate,
                              MidiOutputManager& out)
{
    if (!tmpl || tmpl->bass.isEmpty()) return;

    // ── Adapted path ──────────────────────────────────────────────────────────
    if (adaptedPattern && adaptedPattern->valid)
    {
        const AdaptedStep& as = adaptedPattern->steps[step];
        if (as.muted) return;

        float vel = velForTier(as.velocityTier);

        // Kick velocity tracking on unison hits
        if (as.kickVelTracking > 0.f)
        {
            float humanize  = params.humanizePercent / 100.f;
            float randomVel = velForTier(as.velocityTier);
            vel = vel * (as.kickVelTracking / 127.f) * (1.f - humanize) + randomVel * humanize;
        }

        float velHumanize = (params.humanizePercent / 100.f) * profile.timingToVelocityRatio;
        vel += (random.nextFloat() * 2.f - 1.f) * profile.velocityJitterMax * velHumanize;
        vel += params.velOffset;
        int finalVel = juce::jlimit(1, 127, (int)vel);

        BassArt art     = as.articulation;
        BassArt gateArt = as.gateArt;

        float timingOff    = timingOffsetSamples(art, sampleRate);
        int64 noteOnSample = stepSamplePos + (int64)timingOff;
        float gp           = gatePercent(gateArt);
        int64 noteOff      = noteOnSample + (int64)(currentStepDurSamples * gp);

        if (art == BassArt::SLIDE && lastNoteOnSample >= 0)
        {
            float glide = profile.slideGlideTimeMs * params.glideTimeMs / 100.f;
            noteOnSample = lastNoteOnSample + (int64)(glide * sampleRate / 1000.0);
        }

        int bendVal = 8192, bendOffSamples = 0;
        if (art == BassArt::BEND)
        {
            float normBend = profile.pitchBendRangeSemitones / (float)params.pitchBendRange;
            bendVal = juce::jlimit(0, 16383, 8192 + (int)(normBend * 8191.f));
            bendOffSamples = (int)(profile.pitchBendDurationMs * sampleRate / 1000.0);
        }

        int midiNote = pitchEngine.getNoteForStep(step);
        if (midiNote < 0) midiNote = params.outputRootNote;

        out.scheduleNote(noteOnSample, noteOff,
                         params.outputChannel, midiNote,
                         finalVel, bendVal, bendOffSamples);
        lastNoteOnSample = (int)noteOnSample;
        return;
    }

    // ── Seed-based path (approach B live gating still applies) ────────────────
    auto* bassRow = tmpl->bass[0];
    int velTier   = bassRow->steps[step];
    if (velTier == 0) return;

    BassArt art     = bassRow->timing[step];
    BassArt gateArt = art;
    if (tmpl->bass.size() > 1)
        gateArt = tmpl->bass[1]->timing[step];

    auto lp = tmpl->lockAt(step);

    if (params.liveDrums != nullptr)
    {
        const DrumState& ld = *params.liveDrums;
        if (lp)
        {
            switch (lp->type)
            {
                case LockType::UNISON:
                    if (ld.kickHits[step] == 0) return;
                    break;
                case LockType::ALTERNATE:
                    if (ld.kickHits[step] > 0 || ld.snareHits[step] > 0) return;
                    break;
                case LockType::ANTICIPATE:
                {
                    bool nextKick = false;
                    for (int i = 1; i <= 16; ++i)
                    {
                        int s  = (step + i) & 15;
                        auto lp2 = tmpl->lockAt(s);
                        if (lp2 && lp2->type == LockType::UNISON)
                        {
                            nextKick = ld.kickHits[s] > 0;
                            break;
                        }
                    }
                    if (!nextKick) return;
                    break;
                }
                case LockType::FILL:
                    break;
            }
        }
    }

    if (lp)
    {
        switch (lp->type)
        {
            case LockType::UNISON:    break;
            case LockType::ANTICIPATE:
                if (art == BassArt::GRID) art = BassArt::SLIDE;
                break;
            case LockType::FILL:
                if (velTier > 2) velTier = 2;
                gateArt = BassArt::STACCATO;
                break;
            default: break;
        }
    }

    float vel = velForTier(velTier);

    if (params.liveDrums != nullptr && lp && lp->type == LockType::UNISON)
    {
        float kickVel   = (float)params.liveDrums->kickHits[step];
        float humanize  = params.humanizePercent / 100.f;
        float randomVel = velForTier(velTier);
        vel = vel * (kickVel / 127.f) * (1.f - humanize) + randomVel * humanize;
    }

    float velHumanize = (params.humanizePercent / 100.f) * profile.timingToVelocityRatio;
    vel += (random.nextFloat() * 2.f - 1.f) * profile.velocityJitterMax * velHumanize;
    vel += params.velOffset;
    int finalVel = juce::jlimit(1, 127, (int)vel);

    float timingOff    = timingOffsetSamples(art, sampleRate);
    int64 noteOnSample = stepSamplePos + (int64)timingOff;
    double stepDurSamples = currentStepDurSamples;
    float gp           = gatePercent(gateArt);
    int64 noteOff      = noteOnSample + (int64)(stepDurSamples * gp);

    if (art == BassArt::SLIDE && lastNoteOnSample >= 0)
    {
        float glide = profile.slideGlideTimeMs * params.glideTimeMs / 100.f;
        noteOnSample = lastNoteOnSample + (int64)(glide * sampleRate / 1000.0);
    }

    int bendVal = 8192, bendOffSamples = 0;
    if (art == BassArt::BEND)
    {
        float bendRange = profile.pitchBendRangeSemitones;
        float normBend  = bendRange / (float)params.pitchBendRange;
        bendVal = juce::jlimit(0, 16383, 8192 + (int)(normBend * 8191.f));
        bendOffSamples = (int)(profile.pitchBendDurationMs * sampleRate / 1000.0);
    }

    int midiNote = pitchEngine.getNoteForStep(step);
    if (midiNote < 0) midiNote = params.outputRootNote;

    out.scheduleNote(noteOnSample, noteOff,
                     params.outputChannel, midiNote,
                     finalVel, bendVal, bendOffSamples);
    lastNoteOnSample = (int)noteOnSample;
}

// ─── process ──────────────────────────────────────────────────────────────────

void LockEngine::process(MidiOutputManager& midiOut,
                          const juce::AudioPlayHead::CurrentPositionInfo& pos,
                          double sampleRate,
                          int64 blockStartSample,
                          int numSamples)
{
    if (!tmpl || !pos.isPlaying) return;

    const double beatsPerBar  = 4.0;
    const double stepsPerBeat = 4.0;
    const double bpm          = pos.bpm > 0.0 ? pos.bpm : 120.0;

    currentStepDurSamples = (sampleRate * 60.0) / (bpm * stepsPerBeat);

    auto swingDelaySamples = [&](int step) -> int64 {
        if (step % 2 == 0) return 0;
        float swingFrac = juce::jlimit(0.f, 100.f, params.swingPercent) / 100.f;
        return (int64)(swingFrac * 0.5 * currentStepDurSamples);
    };

    for (int s = 0; s < numSamples; ++s)
    {
        double absPPQ = pos.ppqPosition + (double)s / sampleRate * bpm / 60.0;
        double barPPQ = std::fmod(absPPQ, beatsPerBar);
        int    step   = (int)std::floor(barPPQ * stepsPerBeat) & 15;

        if (step != lastStep)
        {
            if (step == 0)
            {
                int absoluteBar  = (int)(absPPQ / beatsPerBar);
                int phraseBar    = absoluteBar % 8;
                currentPhraseBar = phraseBar;

                PitchEngineParams pp = params.pitch;
                pp.rootMidiNote = params.outputRootNote;

                if (pp.pitchEnabled)
                {
                    if (adaptedPattern && adaptedPattern->valid)
                    {
                        // Build BarPitchState from the adapted roles so pitch tracks movement
                        BarPitchState adaptedState;
                        for (int i = 0; i < 16; ++i)
                        {
                            const AdaptedStep& as = adaptedPattern->steps[i];
                            adaptedState.stepRoles[i]       = as.muted ? PitchRole::NONE : as.pitchRole;
                            adaptedState.stepOctaveOffset[i] = as.octaveOffset;
                        }
                        pitchEngine.computeBarFromState(adaptedState, profile, pp);
                    }
                    else if (expandedPhrase && expandedPhrase->isValid)
                    {
                        pitchEngine.computeBarFromState(expandedPhrase->bars[phraseBar], profile, pp);
                    }
                    else
                    {
                        pitchEngine.computeBar(tmpl, profile, pp);
                    }
                }
                else
                {
                    pitchEngine.computeBar(tmpl, profile, pp);
                }
            }

            lastStep = step;
            int64 stepSample = blockStartSample + s + swingDelaySamples(step);
            processStep(step, stepSample, sampleRate, midiOut);
        }
    }
}
