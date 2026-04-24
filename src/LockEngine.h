#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "GenreProfile.h"
#include "MidiOutputManager.h"
#include "PitchEngine.h"

struct LockEngineParams
{
    float swingPercent       = 55.f;
    float humanizePercent    = 20.f;
    float velOffset          = 0.f;
    float timingOffsetMs     = 0.f;
    float gateLengthScale    = 1.0f;
    float glideTimeMs        = 100.f;
    int   outputChannel      = 2;
    int   outputRootNote     = 36;
    int   pitchBendRange     = 2;

    PitchEngineParams pitch;
};

class LockEngine
{
public:
    LockEngine();

    void setTemplate(const GrooveTemplate* t);
    void setParams(const LockEngineParams& p);

    // Called each processBlock. Returns MIDI events via manager.
    void process(MidiOutputManager& midiOut,
                 const juce::AudioPlayHead::CurrentPositionInfo& pos,
                 double sampleRate,
                 int64 blockStartSample,
                 int   numSamples);

    void reset();

private:
    const GrooveTemplate* tmpl    = nullptr;
    LockEngineParams       params;
    GenreProfile           profile;

    int    lastStep               = -1;
    int    lastNoteOnSample       = -1;
    double currentStepDurSamples  = 0.0;

    PitchEngine pitchEngine;

    mutable juce::Random random;

    void processStep(int step, int64 stepSamplePos, double sampleRate,
                     MidiOutputManager& out);

    float velForTier(int tier) const;
    float timingOffsetSamples(BassArt art, double sampleRate) const;
    float gatePercent(BassArt art) const;
};
