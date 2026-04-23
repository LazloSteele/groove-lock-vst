# Pitch System — Groove Lock VST

## Overview

The original design specified that Groove Lock outputs all bass hits on a single configurable MIDI note, leaving pitch to downstream processing. This document adds an optional pitch generation layer that assigns scale degrees to each step based on the lock type, genre profile, and user-selected scale. When enabled, the plugin outputs melodically coherent basslines. When disabled, it falls back to single-note output as originally specified.

The pitch system is NOT a melody generator. It assigns functional pitch roles — root, fifth, approach tone — based on where each note sits in the groove. The groove template tells you WHEN and HOW to play. The pitch system tells you WHAT note to play at each position.

## Schema additions

### Template schema — new fields

Add to the top level of each template JSON:

```json
{
  "pitch": {
    "densityHint": 2,
    "allowChromaticApproach": true,
    "preferredIntervals": ["root", "b7", "5"],
    "stepHints": [
      { "step": 0, "role": "root" },
      { "step": 6, "role": "5" },
      { "step": 8, "role": "root" },
      { "step": 15, "role": "approach" }
    ]
  }
}
```

| Field | Type | Description |
|-------|------|-------------|
| densityHint | int (1-5) | How many discrete pitches per bar. 1 = root only. 2 = root + one other. 3 = root + two others. 4-5 = full melodic movement. |
| allowChromaticApproach | bool | Whether approach notes can use pitches outside the selected scale (chromatic half-step below target). |
| preferredIntervals | string[] | Ordered list of scale degrees this template favors, from most to least used. |
| stepHints | array | Optional per-step pitch role overrides. If a step has a hint, it takes priority over the lock-type default. |

### Step hint roles

| Role | Meaning | Pitch selection |
|------|---------|-----------------|
| `root` | Tonal anchor | Scale degree 1 |
| `5` | Power/stability | Scale degree 5 |
| `b7` | Funk leading tone | Scale degree b7 (minor 7th) |
| `b3` | Minor character | Scale degree b3 |
| `4` | Suspension/movement | Scale degree 4 |
| `b5` | Blue note / tension | Chromatic — b5 (tritone) regardless of scale |
| `octave` | Energy / punctuation | Root up one octave |
| `approach` | Chromatic approach | Half-step below the NEXT note's pitch |
| `any` | Free choice | Engine picks from preferredIntervals based on what hasn't been used recently |

## Lock type → pitch role mapping

This is the core logic. When a step has no explicit stepHint, the lock type determines the pitch role:

| Lock type | Default pitch role | Rationale |
|-----------|-------------------|-----------|
| **unison** | `root` | When bass and kick hit together, the root grounds the impact. The bass reinforces the kick's fundamental. |
| **alternate** | `5` or `b7` | When the bass fills space the kick leaves, it moves away from root to create melodic interest. Default to 5th on strong beats (1, 5, 9, 13), b7 on weak beats. |
| **anticipate** | `approach` | Approach notes lead into the next hit. Chromatic half-step below the next note's pitch. If chromaticApproach is disabled, use the scale tone below instead. |
| **fill** | `any` | Fill notes are decorative. Pick from preferredIntervals, avoiding whatever pitch was just played. Favor b3, 4, or b7 depending on genre. |
| **no lock point** | Inherit from nearest lock point | Steps that have bass hits but no lock point assigned use the same pitch role as the nearest lock point within 2 steps. If no lock point is nearby, default to `root`. |

## Genre pitch profiles

### G-Funk

```
defaultScale: minor pentatonic (1, b3, 4, 5, b7)
densityRange: 2-4
preferredIntervals: [root, b7, 5, b3, 4]
chromaticApproach: true (but used sparingly — 1 per bar max)
octaveRange: C1-C3 (low register, synth bass territory)
octaveJumps: occasional (1 per 2-4 bars for emphasis)

Notes:
- G-funk bass is melodic but restrained. 2-3 unique pitches per bar is standard.
- The b7 is the defining interval — use it as a leading tone back to root on every turnaround.
- Dorian mode is common (adds natural 6th), but the pitch system should default to minor pentatonic and let the user override to Dorian via scale selection.
- Avoid the b3 on unison lock points — root or 5th only when hitting with the kick.
- Slides between pitches should span 2-3 semitones max.
```

### Mobb

```
defaultScale: natural minor / Aeolian (1, 2, b3, 4, 5, b6, b7)
densityRange: 1-2
preferredIntervals: [root, 5, octave]
chromaticApproach: false (too many notes breaks the starkness)
octaveRange: C1-G2 (very low, sub-heavy)
octaveJumps: rare (1 per 4-8 bars)

Notes:
- Mobb bass is about weight, not melody. Many patterns use root only.
- When a second pitch appears, it's almost always the 5th or octave.
- East Oakland Creep and similar half-time patterns: root on beat 1, root on beat 3. That's it.
- Richmond/Vallejo variants can use b7 as a pickup note but never as a sustain.
- The b6 is available in the scale but almost never used in bass — save it for melodic instruments.
- No chromatic approach tones. Clean intervals only.
```

### Hyphy

```
defaultScale: minor pentatonic (1, b3, 4, 5, b7)
densityRange: 3-5
preferredIntervals: [root, 5, octave, b7, b3, 4]
chromaticApproach: true (quick chromatic runs are part of the bounce)
octaveRange: C1-C3
octaveJumps: frequent (every 1-2 bars — octave jumps are a signature of hyphy bass)

Notes:
- Hyphy bass is the most active melodically. 3-4 unique pitches per bar is standard, up to 5 on busy patterns.
- Octave jumps (root to octave above or below) are THE hyphy bass move. Program them on accent steps.
- Ghost fill notes should use b3 or 4 — they add color without harmonic weight.
- The bounce comes partly from rapid alternation between root and octave/5th — same rhythm, different pitch.
- Chromatic approach notes work well before beat 3 hits — a half-step slide up into the root is classic.
- Staccato notes should prefer root or octave (simple). Legato notes can use any interval.
```

### Wonky

```
defaultScale: Dorian (1, 2, b3, 4, 5, 6, b7) with chromatic passing tones
densityRange: 3-5
preferredIntervals: [root, b7, 5, b3, 4, 6, 2]
chromaticApproach: true (heavy use — wonky bass lives in the cracks)
octaveRange: C1-E3 (wider range than other genres)
octaveJumps: unpredictable (0-3 per bar, varying direction)

Notes:
- Wonky uses the full scale plus chromatic tones. The b5 (tritone) is fair game as a passing tone.
- Dorian is preferred over minor pentatonic because the natural 6th and 2nd add intervallic variety.
- Pitch density can change bar-to-bar within the same template. Bar 1 might use 2 pitches, bar 2 might use 5.
- Approach notes should be chromatic by default — half-step slides from either direction.
- Wide interval leaps (4th, 5th, octave+) are more common than stepwise motion. Wonky bass skips around.
- The 2nd degree (which is the tension note — e.g., G in F minor) is used as a sustained tension pitch. Hold it for 2-4 steps over a chord change for dissonance.
- Pitch bends in wonky can be wider than other genres — full-step bends are acceptable.
- Velocity inversion should correlate with pitch: when bass plays ghost velocity (kick accenting), use more adventurous intervals. When bass accents (kick ghosting), return to root or 5th.
```

### Modern West Coast

```
defaultScale: minor pentatonic (1, b3, 4, 5, b7) with optional Dorian extension
densityRange: 2-4
preferredIntervals: [root, b7, 5, 4, b3]
chromaticApproach: true (used on anticipate steps only)
octaveRange: C1-C3
octaveJumps: occasional (1 per 2-4 bars)

Notes:
- Hybrid approach. Takes g-funk's restraint and adds hyphy's occasional octave jump.
- Default to minor pentatonic. If the user selects Dorian, the 6th becomes available for fills.
- Chromatic approaches only on anticipate lock points — not on fills or alternates.
- The approach note pattern from the Groove Lock templates (bass ghost with slide articulation before a unison hit) should always be a half-step below the target.
- Pitch variety should increase from verse to hook: verse = 2 pitches, hook = 3-4 pitches. This is an arrangement hint, not enforced by the engine, but documented for the user.
```

## Scale definitions

The user selects a root note and a scale type. The engine maps scale degrees to MIDI note numbers.

| Scale name | Intervals (semitones from root) | Common in |
|------------|-------------------------------|-----------|
| Minor pentatonic | 0, 3, 5, 7, 10 | G-Funk, Hyphy, Modern WC |
| Natural minor (Aeolian) | 0, 2, 3, 5, 7, 8, 10 | Mobb |
| Dorian | 0, 2, 3, 5, 7, 9, 10 | Wonky, G-Funk (alt) |
| Blues | 0, 3, 5, 6, 7, 10 | All (adds b5 blue note) |
| Phrygian | 0, 1, 3, 5, 7, 8, 10 | Mobb (dark variant) |
| Chromatic | 0-11 (all semitones) | Wonky (approach tones only) |

The scale selection adds two new parameters:

| Parameter | Range | Default |
|-----------|-------|---------|
| Root note | C-B (0-11) | C |
| Scale type | enum (6 options) | Minor pentatonic |

When the genre changes, the scale type auto-updates to the genre's default. The user can override.

## Pitch engine algorithm

Runs after the LockEngine resolves timing, velocity, and gate for each step. Adds pitch (MIDI note number) to each scheduled event.

```
For each active bass step in the current bar:

1. Determine pitch role:
   a. If the template has a stepHint for this step → use that role
   b. Else if the step has a lock point → use lock-type default role (see table above)
   c. Else → inherit from nearest lock point within 2 steps, or default to "root"

2. Resolve role to scale degree:
   a. "root" → degree 0 (1st)
   b. "5" → degree 4 (5th) in the selected scale
   c. "b7" → degree 6 (minor 7th) in minor pentatonic, or degree 6 in Dorian/Aeolian
   d. "b3" → degree 1 (minor 3rd) in minor pentatonic, or degree 2 in Aeolian
   e. "4" → degree 2 (4th) in minor pentatonic, or degree 3 in Aeolian
   f. "b5" → root + 6 semitones (always chromatic, regardless of scale)
   g. "octave" → root + 12 semitones
   h. "approach" → resolve the NEXT step's pitch first, then subtract 1 semitone (chromatic) or use the scale tone below (diatonic) based on allowChromaticApproach
   i. "any" → pick from preferredIntervals, avoiding the previous step's pitch. Use a simple round-robin or weighted random.

3. Apply octave placement:
   a. Start with the user-selected root note's octave
   b. If role is "octave", go up one octave
   c. If the pitch would be above the genre's octaveRange max, drop an octave
   d. If below the min, raise an octave

4. Apply density limiting:
   a. Track unique pitches used in the current bar
   b. If adding this note would exceed densityHint, substitute root or 5th instead
   c. Exception: approach notes don't count toward density (they're passing tones)

5. Output: MIDI note number added to the scheduled MidiEvent
```

## Pitch density per template

Explicit density values for all 10 existing templates:

| Template | Density | Unique pitches/bar | Notes |
|----------|---------|-------------------|-------|
| Sunset Strip Cruise | 2 | root, b7 | b7 on turnaround only |
| Compton Bounce | 3 | root, 5, octave | Octave on bounce moments |
| East Oakland Creep | 1 | root | Root only. Weight not melody. |
| Richmond Sideshow | 2 | root, approach | Chromatic approach before beat 3 |
| Ghost Ride | 4 | root, 5, octave, b7 | Octave jumps on accents, b7 on ghosts |
| Turf Dance | 2 | root, 5 | Call-response uses only two pitches |
| Glasgow Glitch | 3 | root, 5, b7 | Slides between all three |
| Aqua Crunk | 4 | root, b7, b3, 5 | Velocity inversion: ghost=adventurous, accent=root |
| Modern LA Hybrid | 3 | root, b7, 5 | Approach notes are chromatic |
| Bay Area Slide | 3 | root, 5, 4 | Pitch bends on fill notes add implicit chromaticism |

## Remaining 8 templates — pitch assignments

| Template | Density | Pitches | Strategy |
|----------|---------|---------|----------|
| San Quinn Fog | 2 | root, 5 | Minimal. Ride cymbal interaction is rhythmic, not melodic. |
| Vallejo Lace | 2 | root, b7 | b7 on clave-aligned steps. Long sustains. |
| Dumb Stupid | 3 | root, octave, 5 | Aggressive. Octave on pushed steps. |
| E-40 Bounce | 3 | root, 5, b7 | Laid-back. b7 as pickup note. |
| Parliament Freeway | 4 | root, b7, b3, 5 | Most expressive. All Bootsy intervals. Slides between all. |
| Inglewood Lowrider | 1 | root | Ultra-sparse. Root only. Maximum sustain. |
| LuckyMe Stomp | 4 | root, b7, 5, b3 | Slides on every note. Velocity inversion with kick. |
| Bruk Funk | 3 | root, 5, 4 | Clustered notes share a pitch, then jump for the next cluster. |

## User controls

The pitch system adds a small panel to the GUI sidebar, below the global controls:

```
┌─────────────────────────┐
│  PITCH                  │
│                         │
│  Root: [C ][▼]          │
│  Scale: [Minor Pent][▼] │
│  Density: [●●●○○]      │
│  Chromatic: [on/off]    │
│  Pitch hints: [on/off]  │
│                         │
│  [Octave -] C2 [Oct +]  │
└─────────────────────────┘
```

- **Root:** Dropdown, C through B
- **Scale:** Dropdown, 6 options. Auto-updates when genre changes but user can override.
- **Density:** 1-5 dots. Loaded from template, user-adjustable. Clamped to genre's densityRange.
- **Chromatic:** Toggle. Enables/disables chromatic approach tones.
- **Pitch hints:** Master toggle. When off, all notes output on the single root note (original behavior). When on, the pitch engine runs.
- **Octave:** Base octave for the root. Buttons to shift up/down. Display shows current (e.g., "C2").

## Implementation notes

- The PitchEngine is a new class, called by LockEngine after timing/velocity/gate resolution but before MidiOutputManager.
- It should be stateful within a bar — it tracks which pitches have been used to enforce density limits and avoid repetition.
- State resets at bar boundaries.
- Approach note resolution requires look-ahead: the engine must know the next step's pitch before it can calculate the current approach tone. Process steps in reverse order for approach resolution, then output in forward order.
- The pitch system is entirely optional. All existing templates work without pitch data. If the `pitch` block is missing from a template JSON, the engine defaults to genre profile values.
