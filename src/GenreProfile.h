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

    // Timing
    float defaultTimingOffsetMs     = 0.f;
    float swingMin                  = 0.f;
    float swingMax                  = 100.f;
    float unisonTimingToleranceMs   = 5.f;
    float pushOffsetMs              = -10.f;
    float layOffsetMs               = +12.f;

    // Velocity ranges
    int ghostVelMin = 35, ghostVelMax = 55;
    int medVelMin   = 70, medVelMax   = 90;
    int fullVelMin  = 100, fullVelMax = 115;
    int accentVelMin = 120, accentVelMax = 127;
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

    static GenreProfile forGenre(const juce::String& genre)
    {
        GenreProfile p;
        p.name = genre;

        if (genre == "G-Funk")
        {
            p.defaultScale           = ScaleType::MINOR_PENTATONIC;
            p.pitchDensityMin        = 2; p.pitchDensityMax = 4;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FLAT7, PitchRole::FIFTH,
                                         PitchRole::FLAT3, PitchRole::FOURTH };
            p.bassMidiMin = 24; p.bassMidiMax = 48; // C1-C3
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 8.f;
            p.swingMin = 50.f; p.swingMax = 65.f;
            p.unisonTimingToleranceMs  = 2.f;
            p.pushOffsetMs = -8.f; p.layOffsetMs = 12.f;
            p.ghostVelMin = 45; p.ghostVelMax = 55;
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
        }
        else if (genre == "Mobb")
        {
            p.defaultScale           = ScaleType::NATURAL_MINOR;
            p.pitchDensityMin        = 1; p.pitchDensityMax = 2;
            p.allowChromaticApproach = false;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FIFTH, PitchRole::OCTAVE };
            p.bassMidiMin = 24; p.bassMidiMax = 43; // C1-G2
            p.allowOctaveJumps = false;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 0.f; p.swingMax = 52.f;
            p.unisonTimingToleranceMs  = 0.5f;
            p.pushOffsetMs = -5.f; p.layOffsetMs = 8.f;
            p.ghostVelMin = 35; p.ghostVelMax = 45;
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
        }
        else if (genre == "Hyphy")
        {
            p.defaultScale           = ScaleType::MINOR_PENTATONIC;
            p.pitchDensityMin        = 3; p.pitchDensityMax = 5;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FIFTH, PitchRole::OCTAVE,
                                         PitchRole::FLAT7, PitchRole::FLAT3, PitchRole::FOURTH };
            p.bassMidiMin = 24; p.bassMidiMax = 48;
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = -7.f;
            p.swingMin = 58.f; p.swingMax = 68.f;
            p.unisonTimingToleranceMs  = 5.f;
            p.pushOffsetMs = -8.f; p.layOffsetMs = 10.f;
            p.ghostVelMin = 50; p.ghostVelMax = 60;
            p.accentVelMin = 115; p.accentVelMax = 125;
            p.defaultGatePercent  = 0.55f;
            p.staccatoGatePercent = 0.35f;
            p.legatoGatePercent   = 0.85f;
            p.slideGlideTimeMs    = 60.f;
            p.allowBendsOnPrimaryHits = false;
            p.defaultHumanizePercent  = 0.28f;
            p.timingJitterMaxMs = 5.f;
            p.timingToVelocityRatio = 0.35f;
        }
        else if (genre == "Wonky")
        {
            p.defaultScale           = ScaleType::DORIAN;
            p.pitchDensityMin        = 3; p.pitchDensityMax = 5;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FLAT7, PitchRole::FIFTH,
                                         PitchRole::FLAT3, PitchRole::FOURTH, PitchRole::FLAT5 };
            p.bassMidiMin = 24; p.bassMidiMax = 52; // C1-E3
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 65.f; p.swingMax = 80.f;
            p.unisonTimingToleranceMs  = 12.f;
            p.pushOffsetMs = -15.f; p.layOffsetMs = 18.f;
            p.ghostVelMin = 35; p.ghostVelMax = 65;
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
        }
        else // Modern West Coast (default)
        {
            p.defaultScale           = ScaleType::MINOR_PENTATONIC;
            p.pitchDensityMin        = 2; p.pitchDensityMax = 4;
            p.allowChromaticApproach = true;
            p.preferredIntervals     = { PitchRole::ROOT, PitchRole::FLAT7, PitchRole::FIFTH,
                                         PitchRole::FOURTH, PitchRole::FLAT3 };
            p.bassMidiMin = 24; p.bassMidiMax = 48;
            p.allowOctaveJumps = true;
            p.defaultTimingOffsetMs    = 0.f;
            p.swingMin = 55.f; p.swingMax = 62.f;
            p.unisonTimingToleranceMs  = 3.f;
            p.pushOffsetMs = -12.f; p.layOffsetMs = 10.f;
            p.ghostVelMin = 45; p.ghostVelMax = 55;
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
        }

        return p;
    }
};
