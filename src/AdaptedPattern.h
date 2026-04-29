#pragma once
#include "GrooveTemplate.h"   // BassArt, LockType
#include "PitchTypes.h"       // PitchRole

// One output slot in the adapted pattern. Indexed by OUTPUT step (0-15).
// A slot with muted=true means no bass note fires at that step this bar.
// Slots can be populated by notes that moved from a different seed position.
struct AdaptedStep
{
    int       seedPosition    = -1;           // original seed step (for reference/contour checks)
    int       velocityTier    = 0;            // 0-4
    PitchRole pitchRole       = PitchRole::ROOT;
    BassArt   articulation    = BassArt::GRID;
    BassArt   gateArt         = BassArt::GRID;
    bool      muted           = true;
    int       octaveOffset    = 0;            // from PhraseExpander (0 until integrated)
    float     kickVelTracking = 0.f;          // live kick velocity (0-127) for unison scaling
    bool      ghostFill       = false;        // true = inserted by contour check to prevent empty beat
};

// Full adapted pattern for one bar. When valid=false, LockEngine plays the seed unmodified.
struct AdaptedPattern
{
    AdaptedStep steps[16];
    bool        valid = false;
};
