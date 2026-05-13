#pragma once
#include <JuceHeader.h>
#include "PitchTypes.h"

struct GenreProfile
{
    juce::String name;

    // Pitch
    ScaleType              defaultScale           = ScaleType::MINOR_PENTATONIC;
    int                    pitchDensityMin        = 2;
    int                    pitchDensityMax        = 4;
    bool                   allowChromaticApproach = true;
    juce::Array<PitchRole> preferredIntervals;  // ordered, most to least used
    int                    bassMidiMin            = 24;  // C1
    int                    bassMidiMax            = 48;  // C3
    bool                   allowOctaveJumps       = true;
    bool                   approachFromAbove      = false; // descending half-step into target (Mobb "sneak")
    bool                   unisonForcesRoot       = true;  // when false, unison steps use ANY (Wonky)
    int                    maxApproachRunLength   = 1;     // G-Funk: 3 (2–3 note chromatic run)
    int                    fillResolutionWindow   = 0;     // Hyphy: 2 (max consecutive non-root active steps)

    // Timing
    float defaultTimingOffsetMs     = 0.f;
    float swingMin                  = 0.f;
    float swingMax                  = 100.f;
    float unisonTimingToleranceMs   = 5.f;
    float pushOffsetMs              = -10.f;
    float layOffsetMs               = +12.f;

    // Velocity ranges
    int ghostVelMin = 20, ghostVelMax = 44;
    int medVelMin   = 45, medVelMax   = 69;
    int fullVelMin  = 70, fullVelMax  = 94;
    int accentVelMin = 110, accentVelMax = 127;
    int bassVelocityOffsetFromKick  = 0;
    bool allowVelocityInversion     = false;

    // Gate
    float defaultGatePercent  = 0.65f;
    float staccatoGatePercent = 0.40f;
    float legatoGatePercent   = 0.88f;
    float gateHumanizeRange   = 0.05f;

    // Articulation
    float slideGlideTimeMs         = 80.f;
    float pitchBendRangeSemitones  = 2.f;
    float pitchBendDurationMs      = 75.f;
    bool allowSlidesOnPrimaryHits  = true;
    bool allowBendsOnPrimaryHits   = false;

    // Humanization
    float defaultHumanizePercent   = 0.20f;
    float timingJitterMaxMs        = 5.f;
    float velocityJitterMax        = 8.f;
    float timingToVelocityRatio    = 0.5f;   // 0=timing only, 1=vel only

    // Phrase expansion limits (PHRASE_EXPANSION.md)
    float maxDeviation         = 0.6f;
    float densityClampMin      = 0.2f;
    float densityClampMax      = 0.8f;
    float tensionClampMin      = 0.1f;
    float tensionClampMax      = 0.7f;
    int   maxOctaveDisplPerBar = 1;

    static GenreProfile forGenre(const juce::String& genre)
    {
        GenreProfile p;
        p.name = genre;

        if (genre == "G-Funk")
        {
            p.defaultScale           = ScaleType::NATURAL_MINOR;
            p.pitchDensityMin        = 2; p.pitchDensityMax = 5;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FIFTH, PitchRole::FLAT7,
                                         PitchRole::FOURTH, PitchRole::FLAT3 };
            p.bassMidiMin = 24; p.bassMidiMax = 48; // C1-C3
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 8.f;
            p.swingMin = 50.f; p.swingMax = 65.f;
            p.unisonTimingToleranceMs  = 2.f;
            p.pushOffsetMs = -8.f; p.layOffsetMs = 12.f;
            p.ghostVelMin = 30;  p.ghostVelMax = 45;
            p.medVelMin   = 48;  p.medVelMax   = 66;
            p.fullVelMin  = 72;  p.fullVelMax  = 92;
            p.accentVelMin = 110; p.accentVelMax = 120;
            p.bassVelocityOffsetFromKick = -5;
            p.defaultGatePercent  = 0.85f;
            p.staccatoGatePercent = 0.40f;
            p.legatoGatePercent   = 0.90f;
            p.slideGlideTimeMs    = 120.f;
            p.pitchBendRangeSemitones = 0.5f;
            p.allowBendsOnPrimaryHits = false;
            p.defaultHumanizePercent  = 0.18f;
            p.timingJitterMaxMs = 3.f;
            p.timingToVelocityRatio = 0.3f;
            p.maxDeviation = 0.5f;
            p.densityClampMin = 0.2f; p.densityClampMax = 0.8f;
            p.tensionClampMin = 0.1f; p.tensionClampMax = 0.7f;
            p.maxOctaveDisplPerBar = 1;
            p.maxApproachRunLength = 3;
        }
        else if (genre == "Mobb")
        {
            p.defaultScale           = ScaleType::NATURAL_MINOR;
            p.pitchDensityMin        = 1; p.pitchDensityMax = 4;
            p.allowChromaticApproach = false;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FIFTH, PitchRole::FOURTH,
                                         PitchRole::FLAT3, PitchRole::FLAT5 };
            p.bassMidiMin = 24; p.bassMidiMax = 43; // C1-G2
            p.allowOctaveJumps = false;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 0.f; p.swingMax = 52.f;
            p.unisonTimingToleranceMs  = 0.5f;
            p.pushOffsetMs = -5.f; p.layOffsetMs = 8.f;
            p.ghostVelMin = 18;  p.ghostVelMax = 30;
            p.medVelMin   = 45;  p.medVelMax   = 65;
            p.fullVelMin  = 72;  p.fullVelMax  = 90;
            p.accentVelMin = 120; p.accentVelMax = 127;
            p.bassVelocityOffsetFromKick = 0;
            p.defaultGatePercent  = 0.90f;
            p.staccatoGatePercent = 0.35f;
            p.legatoGatePercent   = 0.95f;
            p.slideGlideTimeMs    = 50.f;
            p.pitchBendRangeSemitones = 0.f;
            p.allowSlidesOnPrimaryHits = false;
            p.allowBendsOnPrimaryHits  = false;
            p.defaultHumanizePercent   = 0.08f;
            p.timingJitterMaxMs = 1.f;
            p.timingToVelocityRatio = 0.8f;
            p.maxDeviation = 0.3f;
            p.densityClampMin = 0.1f; p.densityClampMax = 0.6f;
            p.tensionClampMin = 0.0f; p.tensionClampMax = 0.4f;
            p.maxOctaveDisplPerBar = 0;
            p.approachFromAbove = true;
        }
        else if (genre == "Hyphy")
        {
            p.defaultScale           = ScaleType::MINOR_PENTATONIC;
            p.pitchDensityMin        = 3; p.pitchDensityMax = 9;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::OCTAVE, PitchRole::FLAT7,
                                         PitchRole::FIFTH, PitchRole::FLAT3, PitchRole::FOURTH };
            p.bassMidiMin = 24; p.bassMidiMax = 48;
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = -7.f;
            p.swingMin = 58.f; p.swingMax = 68.f;
            p.unisonTimingToleranceMs  = 5.f;
            p.pushOffsetMs = -8.f; p.layOffsetMs = 10.f;
            p.ghostVelMin = 22;  p.ghostVelMax = 40;
            p.medVelMin   = 46;  p.medVelMax   = 66;
            p.fullVelMin  = 71;  p.fullVelMax  = 90;
            p.accentVelMin = 115; p.accentVelMax = 125;
            p.defaultGatePercent  = 0.55f;
            p.staccatoGatePercent = 0.35f;
            p.legatoGatePercent   = 0.85f;
            p.slideGlideTimeMs    = 60.f;
            p.allowBendsOnPrimaryHits = false;
            p.defaultHumanizePercent  = 0.28f;
            p.timingJitterMaxMs = 5.f;
            p.timingToVelocityRatio = 0.35f;
            p.maxDeviation = 0.7f;
            p.densityClampMin = 0.3f; p.densityClampMax = 1.0f;
            p.tensionClampMin = 0.2f; p.tensionClampMax = 0.8f;
            p.maxOctaveDisplPerBar = 2;
            p.fillResolutionWindow = 2;
        }
        else if (genre == "Wonky")
        {
            p.defaultScale           = ScaleType::DORIAN;
            p.pitchDensityMin        = 3; p.pitchDensityMax = 7;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::SIXTH, PitchRole::FLAT7,
                                         PitchRole::FIFTH, PitchRole::SECOND, PitchRole::FLAT3,
                                         PitchRole::FOURTH, PitchRole::FLAT5 };
            p.bassMidiMin = 24; p.bassMidiMax = 52; // C1-E3
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 65.f; p.swingMax = 80.f;
            p.unisonTimingToleranceMs  = 12.f;
            p.pushOffsetMs = -15.f; p.layOffsetMs = 18.f;
            p.ghostVelMin = 18;  p.ghostVelMax = 52;
            p.medVelMin   = 46;  p.medVelMax   = 68;
            p.fullVelMin  = 70;  p.fullVelMax  = 94;
            p.accentVelMin = 110; p.accentVelMax = 127;
            p.allowVelocityInversion  = true;
            p.defaultGatePercent  = 0.60f;
            p.gateHumanizeRange   = 0.15f;
            p.slideGlideTimeMs    = 130.f;
            p.pitchBendRangeSemitones = 1.0f;
            p.allowSlidesOnPrimaryHits = true;
            p.allowBendsOnPrimaryHits  = true;
            p.defaultHumanizePercent   = 0.40f;
            p.timingJitterMaxMs = 8.f;
            p.timingToVelocityRatio = 0.5f;
            p.maxDeviation = 1.0f;
            p.densityClampMin = 0.2f; p.densityClampMax = 1.0f;
            p.tensionClampMin = 0.2f; p.tensionClampMax = 1.0f;
            p.maxOctaveDisplPerBar = 3;
            p.unisonForcesRoot = false;
        }
        else // Modern West Coast (default)
        {
            p.defaultScale           = ScaleType::MINOR_PENTATONIC;
            p.pitchDensityMin        = 2; p.pitchDensityMax = 6;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FOURTH, PitchRole::FLAT7,
                                         PitchRole::FIFTH, PitchRole::FLAT3 };
            p.bassMidiMin = 24; p.bassMidiMax = 48;
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 55.f; p.swingMax = 62.f;
            p.unisonTimingToleranceMs  = 3.f;
            p.pushOffsetMs = -12.f; p.layOffsetMs = 10.f;
            p.ghostVelMin = 28;  p.ghostVelMax = 44;
            p.medVelMin   = 46;  p.medVelMax   = 66;
            p.fullVelMin  = 72;  p.fullVelMax  = 90;
            p.accentVelMin = 115; p.accentVelMax = 122;
            p.bassVelocityOffsetFromKick = -7;
            p.defaultGatePercent  = 0.85f;
            p.staccatoGatePercent = 0.40f;
            p.legatoGatePercent   = 0.88f;
            p.slideGlideTimeMs    = 100.f;
            p.pitchBendRangeSemitones = 0.25f;
            p.allowBendsOnPrimaryHits = false;
            p.defaultHumanizePercent  = 0.20f;
            p.timingJitterMaxMs = 4.f;
            p.timingToVelocityRatio = 0.4f;
            p.maxDeviation = 0.6f;
            p.densityClampMin = 0.2f; p.densityClampMax = 0.9f;
            p.tensionClampMin = 0.1f; p.tensionClampMax = 0.8f;
            p.maxOctaveDisplPerBar = 1;
        }

        return p;
    }
};
