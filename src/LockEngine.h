#pragma once
#include <JuceHeader.h>
#include "GrooveTemplate.h"
#include "GenreProfile.h"
#include "MidiOutputManager.h"

struct LockEngineParams
{
    float swingPercent       = 55.f;   // 0-100
    float humanizePercent    = 20.f;   // 0-100
    float velOffset          = 0.f;    // -64..+64
    float timingOffsetMs     = 0.f;    // -20..+20
    float gateLengthScale    = 1.0f;   // 0.5..1.5
    float glideTimeMs        = 100.f;
    int   outputChannel      = 2;      // 1-16
    int   outputRootNote     = 36;     // C2
    int   pitchBendRange     = 2;      // semitones
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

    juce::Random random;

    void processStep(int step, int64 stepSamplePos, double sampleRate,
                     MidiOutputManager& out);

    float velForTier(int tier) const;
    float timingOffsetSamples(BassArt art, double sampleRate) const;
    float gatePercent(BassArt art) const;
};
