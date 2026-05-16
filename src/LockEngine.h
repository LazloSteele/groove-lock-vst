#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "GenreProfile.h"
#include "MidiOutputManager.h"
#include "PatternAnalyzer.h"
#include "PitchEngine.h"

struct LockEngineParams
{
    float swingPercent             = 55.f;
    float humanizePercent          = 20.f;
    float timingOffsetMs           = 0.f;
    float gateLengthScale          = 1.0f;
    float glideTimeMs              = 100.f;
    float phraseExpansionDensity   = 0.5f; // 0=sparse(suppress fills) … 1=full
    int   outputChannel      = 2;
    int   outputRootNote     = 36;
    int   pitchBendRange     = 2;

    // When non-null, gates bass steps against live drum input (approach B).
    // Null = template-driven output (no live gating).
    const DrumState* liveDrums = nullptr;

    PitchEngineParams pitch;
};

class LockEngine
{
public:
    LockEngine();

    void setTemplate(const GrooveTemplate* t);
    void setParams(const LockEngineParams& p);

    // Provide the 8-bar phrase for per-bar pitch selection.
    // Pass nullptr to fall back to single-bar template-based pitch.
    void setExpandedPhrase(const ExpandedPhrase* p) { expandedPhrase = p; }

    // Called each processBlock. Returns MIDI events via manager.
    void process(MidiOutputManager& midiOut,
                 const juce::AudioPlayHead::CurrentPositionInfo& pos,
                 double sampleRate,
                 int64 blockStartSample,
                 int   numSamples);

    void reset();

    // Returns which of the 8 phrase bars is currently playing (0-7). Audio thread only.
    int getCurrentPhraseBar() const { return currentPhraseBar; }

private:
    const GrooveTemplate* tmpl    = nullptr;
    LockEngineParams       params;
    GenreProfile           profile;

    const ExpandedPhrase* expandedPhrase     = nullptr; // set from processor

    int    lastStep               = -1;
    int64  lastNoteOnSample       = -1;
    int    currentPhraseBar       = 0;   // 0-7, audio thread only
    int    currentBarVariant      = 0;   // 0=bar1, 1=bar2 (derived from currentPhraseBar)
    double currentStepDurSamples  = 0.0;

    PitchEngine pitchEngine;

    mutable juce::Random random;

    void processStep(int step, int64 stepSamplePos, double sampleRate,
                     MidiOutputManager& out);

    float velForTier(int tier) const;
    float timingOffsetSamples(BassArt art, double sampleRate) const;
    float gatePercent(BassArt art) const;
};
