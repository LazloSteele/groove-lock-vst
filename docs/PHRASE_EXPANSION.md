# 8-Bar Expansion & User Control — Groove Lock VST

## Problem

The current system loops a single bar. Real basslines in hip-hop and funk develop over 4–8 bar phrases — the rhythm stays locked but the pitch content shifts, notes appear and disappear, and the line breathes across the phrase. A one-bar loop sounds like a ringtone. An 8-bar phrase sounds like a bassist playing.

Additionally, every groove is fully deterministic once selected. The user picks a template and gets a fixed output. This means two producers using the same template on the same beat get identical bass MIDI. The system needs user-facing controls that meaningfully change the output while staying within the template's musical constraints.

## Part 1: 8-Bar Phrase Expansion

### Architecture

The 1-bar template remains the **seed pattern** — the rhythmic and velocity skeleton. The 8-bar expansion system applies **per-bar transformations** to the seed, producing 8 variations that share the same groove but differ in pitch content, ghost note placement, and articulation detail.

The 8 bars follow a **phrase arc** — a contour that controls how much the pattern deviates from the seed over time.

### Phrase arc model

Each bar in the 8-bar phrase has a **deviation level** (0.0–1.0) that controls how far it strays from the seed:

```
Bar:       1     2     3     4     5     6     7     8
Arc:     0.0   0.1   0.2   0.4   0.3   0.5   0.7   0.2

         seed  almost same  opening  settle  more   peak   resolve
               same   +ghost up      back   motion
```

This is the default arc. The user can select from arc presets or the arc can be derived from the two user control dimensions (see Part 2).

At deviation 0.0, the bar is an exact copy of the seed. At 1.0, maximum transformation is applied. The transformations are:

### Transformation rules

Applied in order. Each transformation checks the current bar's deviation level to decide whether and how much to act.

#### 1. Pitch degree rotation

The seed pattern assigns pitch roles to each step (root, 5, b7, etc. via the lock-type mapping from PITCH_SYSTEM.md). The 8-bar system rotates which specific degrees fill the non-root roles.

```
Template preferredIntervals: [root, b7, 5, b3, 4]

Bar 1 (dev 0.0): Use seed assignments exactly — root, b7, 5 (if density=3)
Bar 2 (dev 0.1): Same, but swap the FILL step's pitch from b7 to 5
Bar 3 (dev 0.2): Alternate steps use b3 instead of 5
Bar 4 (dev 0.4): Introduce 4th on one fill step. 3 unique pitches → 4 briefly.
Bar 5 (dev 0.3): Return to 3 pitches but use a different set: root, 5, b3
Bar 6 (dev 0.5): b7 returns. One step gets an octave displacement.
Bar 7 (dev 0.7): Maximum pitch variety. All preferredIntervals available. Density temporarily +1.
Bar 8 (dev 0.2): Resolve. Back to seed assignments. Root-heavy. Sets up the next phrase.
```

The rotation follows the `preferredIntervals` list in the template. Each bar walks further into the list as deviation increases. When deviation decreases, it walks back toward the front of the list (root, b7, 5).

**Rule: the root never leaves.** Every bar has root on at least one step (typically unison lock points). The rotation only affects non-root roles.

**Rule: bar 8 always resolves.** Regardless of deviation level, bar 8 uses only root and one other pitch (b7 or 5). This creates a phrase boundary that the ear recognizes as a reset.

#### 2. Ghost note modulation

Ghost notes (velocity tier 1, 35–55) are the most expendable hits in the pattern. The expansion system adds and removes ghosts across the 8 bars to change the rhythmic density without touching the core groove.

```
Deviation 0.0–0.2: Use seed ghost notes exactly as programmed
Deviation 0.2–0.5: Add 1 ghost note on an empty step adjacent to an existing hit
Deviation 0.5–0.8: Add 1–2 ghost notes AND remove 1 existing ghost (swap positions)
Deviation 0.8–1.0: Remove 1–2 ghost notes (strip back for contrast)
```

**Rule: never add ghosts on steps that have drum accents (kick or snare at velocity tier 3+).** Ghosts fill the negative space; they don't compete with the drums.

**Rule: never remove the seed's non-ghost notes.** Only ghost notes (tier 1) are modulated. Medium, full, and accent hits are sacred — they define the groove.

**Rule: added ghosts get staccato gates.** They're passing tones, not structural. Short gate (30–40%) keeps them out of the way.

#### 3. Octave displacement

One of the strongest variation tools in funk bass. A phrase that's been sitting in the C1–C2 range suddenly jumps to C2–C3 for a bar, then drops back. This creates energy without changing the rhythm or the scale degrees.

```
Deviation 0.0–0.3: No octave displacement. Everything in the base octave.
Deviation 0.3–0.6: One note per bar may be displaced up one octave. Choose an accent or fill note, not a unison note.
Deviation 0.6–0.8: Up to 2 notes displaced. Can displace an entire beat (all notes on beat 3, for example).
Deviation 0.8–1.0: Entire bar may shift up one octave, then snap back next bar.
```

**Rule: unison lock point notes stay in the base octave.** When bass and kick hit together, the bass must be in the sub range to lock with the kick. Displacing unison notes up an octave breaks the low-end lock.

**Rule: octave displacement inherits genre limits.** Mobb allows almost no octave displacement (0–1 notes per 8-bar phrase). Hyphy uses it aggressively (up to every bar). See genre limits table below.

#### 4. Articulation variation

The seed defines articulation per step (slide, bend, staccato, legato). The expansion system introduces subtle variation:

```
Deviation 0.0–0.3: Seed articulations exactly
Deviation 0.3–0.5: One "grid" (plain) articulation per bar becomes a slide or bend
Deviation 0.5–0.8: One slide becomes a bend (or vice versa). Adds expressive variety.
Deviation 0.8–1.0: One legato note becomes staccato, or vice versa. Changes the rhythmic feel of that beat.
```

**Rule: anticipate lock points always keep slide articulation.** The approach-note character is structural, not decorative.

**Rule: articulation changes follow genre rules.** Mobb gets almost no articulation variation (maybe 1 change per 8 bars). Wonky gets heavy variation (every bar can change).

#### 5. Turnaround bar (bar 8 special behavior)

Bar 8 has unique rules beyond just low deviation:

- **Pitch:** Root and one other pitch only (b7 preferred for leading-tone pull back to bar 1)
- **Last 2 steps:** If the seed has a note on step 15 or 16, force it to be a slide into the root with ghost velocity. This is the turnaround — the moment that cycles the phrase.
- **If the seed does NOT have a note on step 15–16:** Add one. A ghost-velocity root with slide articulation on step 15. This creates the turnaround even if the seed pattern doesn't have one.
- **Ghost notes:** Strip to minimum. Bar 8 should feel like an exhale before bar 1 inhales again.

### Genre-specific expansion limits

| Genre | Max deviation | Ghost modulation | Octave displacement | Articulation variation | Notes |
|-------|--------------|------------------|---------------------|----------------------|-------|
| G-Funk | 0.5 | +1/−1 per bar | 0–1 notes per phrase | Low | Restraint. The groove barely changes. Subtle pitch rotation is enough. |
| Mobb | 0.3 | +0/−1 per bar | 0–1 notes per phrase | Minimal | Near-static. Bar-to-bar changes should be almost imperceptible. Weight over variety. |
| Hyphy | 0.7 | +2/−2 per bar | 0–2 notes per bar | Moderate | Active. Ghost note patterns can shift significantly. Octave jumps are idiomatic. |
| Wonky | 1.0 | +2/−2 per bar | 0–3 notes per bar | High | Full range. Every bar can feel different while sharing the same skeleton. |
| Modern WC | 0.6 | +1/−1 per bar | 0–1 notes per bar | Moderate | Balanced. More variety than g-funk, more restraint than hyphy. |

### Data structure

The expansion produces an `ExpandedPhrase`:

```cpp
struct ExpandedPhrase {
    std::array<BarState, 8> bars;
    float phraseArc[8];         // deviation level per bar
    int currentBar;             // 0-7, advances with playhead
};

struct BarState {
    // Inherited from seed (may be modified by transformations)
    int stepVelocities[16];     // velocity tier per step (0-4)
    int stepPitchRoles[16];     // pitch role enum per step
    int stepArticulations[16];  // articulation enum per step
    int stepGates[16];          // gate type enum per step
    int stepOctaveOffset[16];   // 0 = base octave, 1 = up one, -1 = down one
    float deviationLevel;       // 0.0–1.0
};
```

The ExpandedPhrase is regenerated:
- When the user selects a new template
- When the user changes either control dimension (see Part 2)
- When the phrase loops (optional: regenerate with different random seeds for perpetual variation)

Regeneration happens on the message thread. The audio thread reads from a double-buffered ExpandedPhrase, swapping at bar 8 → bar 1 boundaries.

---

## Part 2: Two-Dimensional User Control

### Design philosophy

The two dimensions should feel like musical intentions, not technical parameters. A producer should be able to grab them and immediately hear a meaningful change without understanding the underlying system. They should map to decisions a bass player makes instinctively: "should I play more or less?" and "should I play it safe or take risks?"

### Dimension 1: Density (horizontal axis)

**What the user feels:** "How busy is the bassline?"

**Range:** 0.0 (skeletal) to 1.0 (active)

**What it controls:**

At the **rhythmic level:**
- Scales the number of active steps in the seed pattern. At 0.0, only unison lock point steps fire (typically 2–3 notes per bar — the absolute minimum groove). At 1.0, all seed steps fire plus additional ghost fills on empty steps.
- Specifically: steps are ranked by importance. Unison lock points are rank 1 (always play). Alternate and fill lock points are rank 2. Steps with no lock point but non-zero velocity in the seed are rank 3. Empty steps adjacent to active steps are rank 4 (potential ghost fills). The density slider controls the cutoff — low density = only rank 1–2, full density = all ranks including added ghosts.

At the **pitch level:**
- Scales the `densityHint` from the template or pitch profile. At 0.0, force density to 1 (root only). At 1.0, allow the full density from the genre profile.
- The slider position maps to the density value:

```
Slider 0.0–0.2: pitch density 1 (root only)
Slider 0.2–0.4: pitch density 2 (root + one other)
Slider 0.4–0.6: pitch density 3
Slider 0.6–0.8: pitch density 4
Slider 0.8–1.0: pitch density = genre max (up to 5 for wonky)
```

At the **expansion level:**
- Scales the phrase arc's deviation range. At density 0.0, the phrase arc is flat (all bars identical to the seed minus stripped notes). At 1.0, the arc reaches its genre-defined maximum deviation.
- Also controls ghost note modulation intensity: low density = ghosts are removed from the seed, high density = ghosts are added.

**Default position:** 0.5 (produces the seed pattern as authored — the intended groove).

### Dimension 2: Tension (vertical axis)

**What the user feels:** "How adventurous is the pitch and articulation content?"

**Range:** 0.0 (safe) to 1.0 (risky)

**What it controls:**

At the **pitch level:**
- Scales which intervals from `preferredIntervals` are available. At 0.0, only root and 5th are used (the safest, most consonant intervals). At 1.0, the full list is available including b3, 4, b5 (if blues scale), and chromatic approaches.
- Specifically:

```
Slider 0.0–0.2: root, 5 only. Maximum consonance.
Slider 0.2–0.4: root, 5, b7. The b7 adds funk character.
Slider 0.4–0.6: root, 5, b7, b3. Minor tonality fully present.
Slider 0.6–0.8: Full preferredIntervals list. 4th, 6th (if Dorian) available.
Slider 0.8–1.0: Full list + chromatic approach tones + b5 blue note. Maximum tension.
```

At the **articulation level:**
- Scales the intensity of articulations. At 0.0, all notes are plain (grid articulation, no slides or bends). At 1.0, the seed's articulations are fully applied plus additional expression added by the expansion system.
- Slide glide times also scale: at tension 0.0, glide is 0ms (no slide). At 1.0, glide reaches the genre-defined maximum.
- Pitch bend depth scales: at 0.0, no bends. At 1.0, bends reach the genre's maximum range.

At the **expansion level:**
- Scales how much the phrase arc affects articulation and pitch variety. At tension 0.0, even high-deviation bars in the phrase arc won't introduce new intervals. At 1.0, the arc's full pitch rotation and articulation variation are active.
- The bar 7 "peak" behavior (maximum variety) is gated by tension: at low tension, bar 7 sounds almost identical to bar 1. At high tension, bar 7 is the most adventurous bar.

**Default position:** 0.5 (produces the seed pattern's pitch and articulation content as authored).

### Interaction between dimensions

The two dimensions are independent but their effects combine:

| | Low Density (0.0–0.3) | Mid Density (0.3–0.7) | High Density (0.7–1.0) |
|---|---|---|---|
| **Low Tension (0.0–0.3)** | Minimal. 2 notes/bar, root and 5th only, no articulation. Ultra-sparse sub-bass pulse. | Seed rhythm, safe pitches (root, 5, b7), plain articulation. Clean and simple. | Busy rhythm, but all root and 5th. Rhythmically active, harmonically static. Hyphy-style bounce with no melodic risk. |
| **Mid Tension (0.3–0.7)** | Sparse rhythm but with slides, bends, and minor tonality (b3). Melodic with space. | **The seed pattern as authored.** This is the default. | Busy rhythm with full pitch variety and articulation. Active and expressive. Bootsy territory. |
| **High Tension (0.7–1.0)** | Very few notes but each one is expressive — wide bends, chromatic approaches, unexpected intervals. Minimalist but tense. | Seed rhythm with maximum pitch adventure — b5 blue notes, chromatic passing tones, full articulation. Wonky feel even on non-wonky templates. | Maximum everything. Busy, chromatic, heavily articulated. Approaching jazz-funk territory. Will sound unhinged on mobb templates. |

### Genre clamping

The raw slider values are clamped per genre so the user can't push a mobb template into jazz-funk territory (unless they really want to — see override below):

| Genre | Density clamp | Tension clamp |
|-------|--------------|---------------|
| G-Funk | 0.2–0.8 | 0.1–0.7 |
| Mobb | 0.1–0.6 | 0.0–0.4 |
| Hyphy | 0.3–1.0 | 0.2–0.8 |
| Wonky | 0.2–1.0 | 0.2–1.0 |
| Modern WC | 0.2–0.9 | 0.1–0.8 |

A "clamp override" toggle in the settings (not the main GUI — buried intentionally) disables genre clamping and gives the user the full 0.0–1.0 range on both axes. This lets you do things like apply wonky-level tension to a mobb template, which will sound weird but might be exactly what someone wants.

### GUI implementation

The two dimensions are presented as an **XY pad** — a single 2D control surface that the user can click/drag to set both values simultaneously.

```
┌───────────────────────────┐
│            TENSION         │
│  ▲                         │
│  │    ·                    │
│  │         ● (cursor)      │
│  │                    ·    │
│  │              ·          │
│  └──────────────────► ──── │
│          DENSITY           │
│                            │
│  [0.45, 0.52]  (readout)   │
└───────────────────────────┘
```

- 200x200px area in the sidebar, replacing or supplementing the individual knobs
- X axis = Density (left=sparse, right=busy)
- Y axis = Tension (bottom=safe, top=risky)
- Current position shown as a dot with crosshairs
- Coordinate readout below the pad
- Double-click to reset to center (0.5, 0.5)
- The pad background can subtly shade to indicate the genre's clamped range (the accessible area is brighter, the clamped-out corners are darker)
- Both values are automatable parameters in the DAW

The individual knobs for swing, humanization, gate scale, etc. remain — the XY pad doesn't replace them. It sits above or below the knob grid as the primary creative control.

### Parameter integration

| Parameter | Type | Range | Default | Automatable |
|-----------|------|-------|---------|-------------|
| Density | float | 0.0–1.0 | 0.5 | Yes |
| Tension | float | 0.0–1.0 | 0.5 | Yes |
| Genre clamp override | bool | on/off | off | No |
| Phrase regeneration mode | enum | fixed / per-loop / manual | per-loop | No |

**Phrase regeneration modes:**
- **fixed:** The 8-bar phrase is generated once when the template loads or controls change. It loops identically.
- **per-loop:** Each time the phrase cycles (bar 8 → bar 1), a new variation is generated with different random seeds for ghost placement, pitch rotation starting point, and octave displacement choices. The groove evolves over time. All random choices are still bounded by the density and tension values.
- **manual:** The phrase is generated once and holds. User clicks a "Regenerate" button to get a new variation. Good for auditioning options and committing to one.

---

## Part 3: Pitch Degree Procedural Rules

These are the specific rules the PitchEngine uses when expanding the 1-bar seed to 8 bars. They codify the bassline music theory for groove-centered, less-melody-driven music.

### Rule 1: Root gravity

The root pitch exerts gravitational pull on the phrase. The further a bar gets from bar 1, the more the root's dominance can relax — but it must reassert at phrase boundaries.

```
Bars 1, 2: Root on at least 60% of active steps
Bars 3, 4: Root on at least 40% of active steps
Bars 5, 6: Root on at least 30% of active steps (minimum — even at max tension)
Bar 7:     Root on at least 25% of active steps (the most free bar)
Bar 8:     Root on at least 70% of active steps (resolve)
```

"Active steps" means steps with velocity tier > 0. These percentages are floors — the density and tension sliders can keep root usage higher but never push it below these minimums.

### Rule 2: Interval distance from root correlates with rhythmic weakness

Strong beats (steps 1, 5, 9, 13 — the four quarter notes) prefer intervals close to root: root itself, 5th, or octave. These are the stable, consonant intervals that anchor the groove.

Weak beats (offbeat 16th notes — steps 2, 4, 6, 8, 10, 12, 14, 16) are where the b7, b3, 4th, and chromatic tones live. This is fundamental to how funk bass works — Bootsy plays root on the beat, then decorates between beats.

```
Strong beat (steps 1, 5, 9, 13):
  Tension 0.0–0.5: root or 5 only
  Tension 0.5–0.8: root, 5, or octave
  Tension 0.8–1.0: root, 5, octave, or b7

Weak beat (all other steps):
  Tension 0.0–0.3: root or 5
  Tension 0.3–0.6: any from preferredIntervals
  Tension 0.6–1.0: any from preferredIntervals + chromatic approaches
```

### Rule 3: Stepwise motion over leaps

When two adjacent active steps both have non-root pitches, the interval between them should prefer steps (1–2 scale degrees apart) over leaps (3+ degrees apart). This creates smooth, singable lines even at high density.

```
Exception: octave jumps. Root → octave is always allowed regardless of the stepwise preference. It's a leap but it's functionally a unison — same pitch class, different register. Hyphy bass is built on this move.

Exception: bar 7 at high tension. The "peak" bar can use wider leaps (4th, 5th) for dramatic effect.

Exception: after a rest. If there's an empty step (vel 0) between two notes, the leap restriction doesn't apply — the rest resets the ear's expectation.
```

### Rule 4: Approach note resolution

Every approach note (chromatic half-step) MUST resolve to its target on the next active step. An approach note that doesn't resolve sounds like a wrong note.

```
If step N has role "approach":
  Step N+1 (or the next active step after N) must be root, 5, b7, or whatever the approach was targeting
  The approach pitch = target pitch minus 1 semitone (chromatic) or minus the nearest scale tone (diatonic)
  The approach note must have lower velocity than the target (ghost or med vs. full or accent)
  The approach note must have shorter gate than the target (staccato vs. legato)
```

If the expansion system places an approach note but the next step has no bass hit, the approach is cancelled and replaced with the previous step's pitch role. Dangling approach tones are not allowed.

### Rule 5: Repetition before variation

Within a single bar, if the same pitch role appears on two non-adjacent steps, it should be the same actual pitch (same octave). This creates motivic consistency — the ear hears "that's the same note coming back" rather than "random pitches."

Across bars, the SAME steps should tend to keep the SAME pitch role for 2–3 bars before rotating. This creates the sense that the bassline has a motif that evolves, not a new random melody every bar.

```
Example across 4 bars (showing step 6 only):
  Bar 1: step 6 = b7
  Bar 2: step 6 = b7 (same — repetition)
  Bar 3: step 6 = 5  (changed — variation after 2 bars of repetition)
  Bar 4: step 6 = 5  (same — new repetition)
```

The rotation period (how many bars before a step's pitch role changes) is controlled by tension:

```
Tension 0.0–0.3: roles hold for 4 bars before rotating
Tension 0.3–0.6: roles hold for 2–3 bars
Tension 0.6–0.8: roles hold for 1–2 bars
Tension 0.8–1.0: roles can change every bar
```

### Rule 6: The b7 turnaround

In bars 4 and 8 (phrase midpoint and endpoint), the last active step of the bar should be b7 resolving down to root on beat 1 of the next bar. This is the single most common bass move in funk and hip-hop — the minor 7th as a leading tone pulling back home.

```
Bar 4, last active step: b7 (with slide articulation into bar 5's root)
Bar 8, last active step: b7 (with slide articulation into bar 1's root)
```

If the seed pattern doesn't have a note on the last steps of bar 4 or 8, add a ghost note with slide articulation. This turnaround should exist even at minimum density.

At tension 0.0–0.2, the turnaround note is root (no b7). At 0.2+, it's b7. This way, the lowest-tension setting produces a completely consonant loop with no leading-tone pull, which some producers might want for very static, droning bass.

### Rule 7: Octave register as energy

The octave register of the bass communicates energy level. Lower = heavier, more grounded. Higher = more energetic, more exposed.

```
Bars 1–2: base octave (e.g., C1–C2). Establishing.
Bars 3–4: base octave with occasional +1 octave on accent notes. Building.
Bars 5–6: mixed — some notes at base, some at +1. Peak energy approaching.
Bar 7: most notes at +1 octave. Peak energy. The bass is "singing."
Bar 8: back to base octave. Resolving. Grounding.
```

This octave contour is scaled by density:
- At density 0.0–0.3, everything stays in base octave regardless of bar position.
- At density 0.5, the contour above applies.
- At density 0.8–1.0, the contour is more extreme — bar 7 might go to +2 octave for hyphy/wonky styles.

---

## Implementation summary

### New/modified classes

**PhraseExpander** (new class):
- Takes a seed BarState + GenreProfile + Density + Tension
- Produces an ExpandedPhrase (8 BarStates)
- Applies all transformation rules and procedural pitch rules
- Called on message thread, result double-buffered to audio thread

**PitchEngine** (modified):
- Now receives BarState from the ExpandedPhrase instead of directly from the template
- Pitch role → MIDI note resolution unchanged
- Density and tension values passed through for the interval availability rules

**LockEngine** (modified):
- Now iterates over 8 bars instead of looping 1
- Tracks current bar index (0–7) from playhead position
- Reads the appropriate BarState from the ExpandedPhrase

**PluginProcessor** (modified):
- Adds Density and Tension as AudioProcessorParameters
- Manages phrase regeneration based on mode (fixed/per-loop/manual)
- Handles double-buffering of ExpandedPhrase between threads

### New parameters

| Parameter | Range | Default | Automatable |
|-----------|-------|---------|-------------|
| Density | 0.0–1.0 | 0.5 | Yes |
| Tension | 0.0–1.0 | 0.5 | Yes |
| Phrase regen mode | fixed/per-loop/manual | per-loop | No |
| Genre clamp override | on/off | off | No |

### GUI additions

- XY pad (200x200px) in sidebar above the knob grid
- "Regenerate" button visible when regen mode = manual
- 8-bar position indicator in the transport bar (which bar of the phrase is currently playing)
- Optional: phrase arc visualization — 8 small vertical bars showing the deviation level, visible below the XY pad
