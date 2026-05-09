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

// ─── processStep ──────────────────────────────────────────────────────────────

void LockEngine::processStep(int step, int64 stepSamplePos, double sampleRate,
                              MidiOutputManager& out)
{
    if (!tmpl || tmpl->bass.isEmpty()) return;

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
                    if (params.alternateMuteOnHat && ld.hatHits[step] > 0) return;
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
                    if (expandedPhrase && expandedPhrase->isValid)
                        pitchEngine.computeBarFromState(expandedPhrase->bars[phraseBar], profile, pp);
                    else
                        pitchEngine.computeBar(tmpl, profile, pp);
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
