# Genre Rules — Groove Lock VST

## Overview

When the LockEngine generates bass MIDI, it adjusts its behavior based on the genre of the selected template. These rules are applied on top of the template data to ensure genre-appropriate output even when the user adjusts global parameters like swing and humanization.

The genre is read from `meta.genre` in the template JSON. If the genre doesn't match any known genre, fall back to "Modern West Coast" rules.

## G-Funk

### Timing character
- Everything slightly behind the grid. Default timing offset: +5ms to +12ms on all bass hits.
- Swing is moderate (50-62%). Never exceed 65% — it stops sounding like g-funk and starts sounding like wonky.
- Kick and bass should be within 2ms of each other on unison lock points. G-funk pocket is tight on the low end.

### Velocity character
- Dynamic range is moderate. Ghost notes at 45-55, accents at 110-120. Avoid extreme velocity contrasts.
- Bass accents should never exceed kick accent velocity. The kick leads, bass follows.

### Gate character
- Default to legato gates (80-90%). G-funk bass sustains and breathes.
- Staccato only on pickup/turnaround notes (typically the last step of the bar).

### Articulation character
- Slides are slow (100-150ms glide time). Smooth, not snappy.
- Pitch bends are subtle — quarter-step to half-step maximum. The bend should be felt, not heard as a distinct pitch change.
- Minimal use of staccato or percussive articulations.

### Humanization
- Low humanization (10-25%). G-funk is programmed but smooth. Too much randomization breaks the cruise feel.

## Mobb

### Timing character
- Straight or nearly straight timing. Swing 0-52% maximum.
- On half-time patterns (snare on 3), bass should hit with absolutely zero timing offset on beat 1 and beat 3. Mechanical precision IS the aesthetic.
- Ghost notes and fills can have slight laid-back offset (+5-10ms) but primary hits must be grid-locked.

### Velocity character
- Extreme contrast. Ghost notes at 35-45, accents at 120-127. The difference between the quietest and loudest hit should be dramatic.
- Bass and kick should be nearly identical velocity on unison points. They function as one instrument.

### Gate character
- Long, sustained gates. Legato default (85-100%). The 808 kick tail and bass sustain overlap — this is intentional and managed via sidechain, not gate length.
- No staccato except on the very rare fill note.

### Articulation character
- Minimal articulation. No slides, no bends. Clean note-on, clean note-off. The menace comes from simplicity and weight, not expression.
- Exception: a single slide approach note before beat 3 (if present in the template) can use a fast slide (40-60ms).

### Humanization
- Very low (0-15%). Mobb is deliberately mechanical. Humanization should affect velocity more than timing — small velocity jitter is okay, timing jitter is not.

## Hyphy

### Timing character
- Active swing (58-68%). The bounce lives in the swing.
- Bass notes that coincide with kick hits should be slightly pushed early (-5 to -10ms) while the kick stays on grid. This creates the "leaning forward" energy that drives hyphy.
- Hat-aligned bass ghost notes should match the hat swing exactly.

### Velocity character
- Bass ghost notes are important and should be clearly audible (50-60 velocity). They participate in the bounce — too quiet and the 16th-note energy collapses.
- Accent contrast is moderate (accents 115-125). Hyphy is energetic but not as dynamics-extreme as mobb.

### Gate character
- Mix of staccato and legato within the same bar. The alternation between short and long gates is part of the bounce.
- Default: staccato on offbeat 16ths (30-40%), legato on downbeat hits (80-90%).

### Articulation character
- Slides are fast (40-80ms). Snappy, not smooth.
- Pitch bends are acceptable on specific fill notes but not on primary hits.
- Staccato markers on ghost notes and fill notes keep the pattern tight and bouncy.

### Humanization
- Moderate (20-35%). Hyphy can be slightly loose — it should feel energetic and human, but not sloppy. Apply humanization more to velocity than timing.

## Wonky

### Timing character
- Deep swing (65-80%). The grid is elastic.
- Kick and bass should NOT be tightly locked on timing — allow 5-15ms of independent drift between them even on unison points. This looseness is the genre.
- Per-step timing offsets should be larger and more varied than other genres. Steps can be pushed or laid back by 10-20ms.

### Velocity character
- Extreme and unpredictable. Velocity inversion is common — when kick accents, bass ghosts, and vice versa. The LockEngine should support this on unison lock points.
- Ghost notes can be very quiet (35-40) or moderately loud (55-65) — vary within the same bar.

### Gate character
- Varied within each bar. Mix staccato, normal, and legato unpredictably.
- Gate length itself can be a humanization target — randomize gate percentage by +/- 15%.

### Articulation character
- Heavy use of slides (every note can potentially slide). Glide time varies: 60-200ms.
- Pitch bends are acceptable on any note, with wider range (half-step to full-step).
- Found-sound-style percussion in the bass register: very short staccato notes with pitch bend can simulate this.

### Humanization
- High (30-50%). Wonky should feel organic, off-kilter, and slightly chaotic while still grooving. Apply humanization to both timing and velocity aggressively.

## Modern West Coast

### Timing character
- Moderate swing (55-62%). The hybrid zone.
- Kick-bass timing should be tight (within 3ms) on unison points — modern production values.
- Chromatic approach notes (anticipate lock points) should be clearly early (-10 to -15ms).

### Velocity character
- Polished dynamic range. Ghosts at 45-55, accents at 115-122. Not extreme in either direction.
- Bass velocity should be consistently 5-10 points below kick velocity on shared hits. Clean, produced relationship.

### Gate character
- Legato default (80-90%) on primary hits. Staccato on approach/anticipation notes.
- Gate transitions should be smooth — avoid abrupt alternation between very short and very long gates in the same bar.

### Articulation character
- Slides on approach notes (80-120ms, moderate speed).
- Subtle pitch bends (+/- quarter-step) on select notes for expression.
- Overall cleaner and more restrained than wonky, more expressive than mobb.

### Humanization
- Low to moderate (15-25%). Modern West Coast sounds produced and intentional. Enough humanization to feel alive, not enough to feel loose.

## Implementation

These rules should be implemented as a `GenreProfile` struct or class:

```cpp
struct GenreProfile {
    juce::String name;

    // Timing
    float defaultTimingOffsetMs;     // global offset applied to all bass hits
    float swingMin, swingMax;        // valid swing range for this genre
    float unisonTimingToleranceMs;   // how tight kick-bass lock should be
    float pushOffsetMs;              // how far "pushed" notes go early
    float layOffsetMs;               // how far "laid back" notes go late

    // Velocity
    int ghostVelMin, ghostVelMax;
    int medVelMin, medVelMax;
    int fullVelMin, fullVelMax;
    int accentVelMin, accentVelMax;
    int bassVelocityOffsetFromKick;  // negative = bass quieter than kick
    bool allowVelocityInversion;     // wonky only

    // Gate
    float defaultGatePercent;
    float staccatoGatePercent;
    float legatoGatePercent;
    float gateHumanizeRange;         // +/- percentage for randomization

    // Articulation
    float slideGlideTimeMs;
    float pitchBendRangeSemitones;
    float pitchBendDurationMs;
    bool allowSlidesOnPrimaryHits;
    bool allowBendsOnPrimaryHits;

    // Humanization
    float defaultHumanizePercent;
    float timingJitterMaxMs;
    float velocityJitterMax;         // +/- velocity units
    float timingToVelocityRatio;     // 0=timing only, 1=velocity only, 0.5=equal
};
```

Populate one `GenreProfile` per genre. The LockEngine reads the profile for the current template's genre and uses it to modulate all output calculations. User-facing parameters (swing knob, humanization knob, etc.) are clamped or scaled according to the genre profile's valid ranges.
