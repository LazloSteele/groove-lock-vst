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

void LockEngine::processStep(int step, int64 stepSamplePos, double sampleRate,
                              MidiOutputManager& out)
{
    if (!tmpl || tmpl->bass.isEmpty()) return;

    const int barVariant = currentBarVariant;
    const auto& bassSource = (barVariant == 1) ? tmpl->bass2 : tmpl->bass;

    auto* bassRow = bassSource[0];
    int velTier   = bassRow->steps[step];
    if (velTier == 0) return;

    BassArt art     = bassRow->timing[step];
    BassArt gateArt = art;
    if (bassSource.size() > 1)
        gateArt = bassSource[1]->timing[step];

    auto lp = tmpl->lockAt(step, barVariant);

    // Live drum gating: when liveDrums is set, suppress or allow steps per lock type
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
                    // Fire only if the next UNISON lock step will have a kick hit
                    bool nextKick = false;
                    for (int i = 1; i <= 16; ++i)
                    {
                        int s = (step + i) & 15;
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
                    break; // fire freely
            }
        }
        // No lock point: fire as normal (safe default)
    }

    if (lp)
    {
        switch (lp->type)
        {
            case LockType::UNISON:
                // Tighten timing — reduce jitter to near-zero
                break;
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

    // Density gate: suppress fill/alternate notes at low density so the Dense
    // axis can pull a busy seed pattern back to a skeleton.
    if (params.phraseExpansionDensity < 0.4f && lp)
    {
        const bool suppressible = (lp->type == LockType::FILL || lp->type == LockType::ALTERNATE);
        if (suppressible)
        {
            // suppression: 0.4→0.0, 0.2→0.5, 0.0→1.0 — gate is silent at 0.4
            const float suppression = 1.f - params.phraseExpansionDensity / 0.4f;
            if (velTier == 1 && suppression > 0.05f) return; // ghost fills first
            if (velTier == 2 && suppression > 0.55f) return; // medium fills below ¼
        }
    }

    // Velocity
    float vel = velForTier(velTier);

    // UNISON velocity matching: scale bass velocity to track the live kick velocity
    if (params.liveDrums != nullptr && lp && lp->type == LockType::UNISON)
    {
        float kickVel  = (float)params.liveDrums->kickHits[step];
        float humanize = params.humanizePercent / 100.f;
        float randomVel = velForTier(velTier);
        vel = vel * (kickVel / 127.f) * (1.f - humanize) + randomVel * humanize;
    }
    float velHumanize = (params.humanizePercent / 100.f) * profile.timingToVelocityRatio;
    vel += (random.nextFloat() * 2.f - 1.f) * profile.velocityJitterMax * velHumanize;
    int finalVel = juce::jlimit(1, 127, (int)vel);

    // Timing
    float timingOff   = timingOffsetSamples(art, sampleRate);
    int64 noteOnSample = stepSamplePos + (int64)timingOff;

    // Step duration in samples (16th note at current BPM)
    // We approximate using a cached sampleRate and BPM from the caller;
    // for prototype just use a placeholder — real value injected via bpm param
    // (LockEngine::process sets this before calling processStep)
    double stepDurSamples = currentStepDurSamples;

    // Gate
    float gp        = gatePercent(gateArt);
    int64 noteOff   = noteOnSample + (int64)(stepDurSamples * gp);

    // Slide: overlap with previous note
    if (art == BassArt::SLIDE && lastNoteOnSample >= 0)
    {
        float glide = profile.slideGlideTimeMs * params.glideTimeMs / 100.f;
        noteOnSample = lastNoteOnSample + (int64)(glide * sampleRate / 1000.0);
    }

    // Pitch bend
    int bendVal         = 8192;
    int bendOffSamples  = 0;
    if (art == BassArt::BEND)
    {
        float bendRange = profile.pitchBendRangeSemitones;
        float normBend  = bendRange / (float)params.pitchBendRange;
        bendVal = 8192 + (int)(normBend * 8191.f);
        bendVal = juce::jlimit(0, 16383, bendVal);
        bendOffSamples = (int)(profile.pitchBendDurationMs * sampleRate / 1000.0);
    }

    int midiNote = pitchEngine.getNoteForStep(step);
    if (midiNote < 0) midiNote = params.outputRootNote;

    out.scheduleNote(noteOnSample, noteOff,
                     params.outputChannel, midiNote,
                     finalVel, bendVal, bendOffSamples);

    lastNoteOnSample = noteOnSample;
}

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

    // Swing delay on odd steps (1,3,5,...): delay by swing% of half a 16th note
    auto swingDelaySamples = [&](int step) -> int64 {
        if (step % 2 == 0) return 0;
        float swingFrac = juce::jlimit(0.f, 100.f, params.swingPercent) / 100.f;
        return (int64)(swingFrac * 0.67 * currentStepDurSamples);
    };

    for (int s = 0; s < numSamples; ++s)
    {
        double absPPQ = pos.ppqPosition + (double)s / sampleRate * bpm / 60.0;
        double barPPQ = std::fmod(absPPQ, beatsPerBar);
        int    step   = (int)std::floor(barPPQ * stepsPerBeat) & 15;

        if (step != lastStep)
        {
            // Recompute pitch at the start of each bar
            if (step == 0)
            {
                int absoluteBar   = (int)(absPPQ / beatsPerBar);
                int phraseBar     = absoluteBar % 8;
                currentPhraseBar  = phraseBar;
                currentBarVariant = (phraseBar % 2 == 1 && tmpl && !tmpl->bass2.isEmpty()) ? 1 : 0;

                PitchEngineParams pp = params.pitch;
                pp.rootMidiNote = params.outputRootNote;
                pp.barVariant   = currentBarVariant;

                if (pp.pitchEnabled)
                {
                    if (expandedPhrase && expandedPhrase->isValid)
                    {
                        const BarPitchState& activeState =
                            (currentBarVariant == 1 && expandedPhrase->hasBar2)
                                ? expandedPhrase->bars2[phraseBar]
                                : expandedPhrase->bars[phraseBar];
                        pitchEngine.computeBarFromState(activeState, profile, pp);
                    }
                    else
                        pitchEngine.computeBar(tmpl, profile, pp);
                }
                else
                    pitchEngine.computeBar(tmpl, profile, pp);
            }

            lastStep = step;
            int64 stepSample = blockStartSample + s + swingDelaySamples(step);
            processStep(step, stepSample, sampleRate, midiOut);
        }
    }
}
