# Architecture — Groove Lock VST

## Overview

Groove Lock is a MIDI effect plugin. It receives drum MIDI, analyzes it against an internal 16-step grid, applies interlocking groove rules from a template, and outputs bass MIDI. No audio is processed.

## Threading model

JUCE plugins have two threads that matter:

- **Audio thread** (`processBlock`): receives and sends MIDI. Must be lock-free, no allocations, no blocking. All MIDI processing happens here.
- **Message thread** (GUI/timer): handles UI rendering, user interaction, and phrase regeneration. Can allocate, block, etc.

Communication between threads uses lock-free structures only:
- Audio → UI: `juce::Atomic<int>` for step position, current phrase bar
- UI → Audio: `juce::Atomic<int/float>` for all parameters (swing, density, tension, etc.)

**Template pointer:** the browser owns all templates (read-only, lifetime = plugin lifetime). The processor stores an `std::atomic<const GrooveTemplate*>` which the message thread writes and the audio thread reads. No copy needed.

**Expanded phrase:** double-buffered. The processor owns `ExpandedPhrase phraseBuffers[2]`. The message thread always writes to the inactive buffer (`phraseBuffers[inactiveBuffer]`), then does a release store of the buffer index. The audio thread does an acquire load of the index each processBlock and passes the resulting pointer to LockEngine via `syncParams()`. This guarantees the audio thread never reads a half-written phrase.

## Core classes

### GrooveTemplate

```
Data structure:
- name: String
- genre: String
- region: String
- mood: String
- tempoMin, tempoMax: float
- swingPercent: float
- description: String

- drumPattern: array[5] of DrumRow
  - DrumRow: label (String), steps (int[16] velocity tier), timing (int[16] timing type enum)

- bassPattern: array[2] of BassRow
  - BassRow: label (String), steps (int[16] velocity tier), timing (int[16] articulation enum)

- lockPoints: array of LockPoint
  - LockPoint: step (int 0-15), type (enum: UNISON/ALTERNATE/ANTICIPATE/FILL), description (String)
```

Serialization: JSON via `juce::JSON`. One file per template. See TEMPLATE_SCHEMA.md.

The template defines WHEN to play (which steps), HOW HARD (velocity tier), HOW LONG (gate percentage), and WITH WHAT ARTICULATION (slide, bend, staccato, legato).

The optional `pitch` block in a template adds:
- `densityHint` (int 1–5): how many unique pitches per bar
- `allowChromaticApproach` (bool): whether approach tones can be half-steps outside the scale
- `preferredIntervals` (string[]): ordered list of scale degrees this template favors
- `stepHints` (array): per-step pitch role overrides that take priority over lock-type defaults

If the pitch block is absent, the genre profile's defaults are used.

### PatternAnalyzer

Responsible for mapping incoming drum MIDI to the internal 16-step grid.

```
Input: raw MIDI note-on messages from the DAW
Output: a DrumState structure updated in real-time

DrumState:
- currentStep: int (0-15, derived from playhead position)
- kickHits[16]: velocity (0 if no hit)
- snareHits[16]: velocity
- hatHits[16]: velocity
- percHits[16]: velocity (catch-all for other drums)
```

**Step calculation from playhead:**
```
beatsPerBar = timeSignature.numerator (assume 4)
stepsPerBeat = 4 (16th notes)
stepsPerBar = 16
currentStep = floor((playheadPositionInBeats % beatsPerBar) * stepsPerBeat)
```

**Drum classification by MIDI note number:**
User-configurable mapping, defaults to General MIDI drum map:
- Kick: note 36
- Snare: notes 38, 40
- Closed hat: note 42
- Open hat: note 46
- Clap: note 39
- Rimshot: note 37
- Tom high: note 50
- Tom mid: note 47
- Tom low: note 45
- Cowbell: note 56
- Crash: note 49
- Ride: note 51

The analyzer quantizes incoming hits to the nearest 16th-note step but records the timing offset (how far from the grid the hit actually was, in ms). This offset data is used to match the bass output's timing feel to the incoming drums.

**Input modes:**
1. **Live input mode:** Analyzer reads MIDI from the plugin input in real-time. Best for producers who have drum MIDI on a track already.
2. **Internal pattern mode:** No drum MIDI input. The plugin uses the drum pattern from the selected groove template. The user edits the pattern in the GUI. Best for building from scratch.

### LockEngine

The core algorithm. Takes a GrooveTemplate and produces bass MIDI events.

```
Input:
- GrooveTemplate (selected preset or user-edited)
- Current tempo (BPM)
- Current playhead position (beats)
- Global swing amount (0-100%)
- Humanization amount (0-100%)
- User-set parameters (output channel, velocity offset, timing offset)

Output:
- Array of MidiEvent to send in the current processBlock
```

**Per-step processing (runs once per step transition):**

```
1. Determine current step index from playhead
2. Look up bass pattern at this step
3. If velocity tier == OFF, skip
4. Resolve velocity:
   a. Get tier range (e.g., GHOST = 35-55)
   b. Randomize within range (scaled by humanization amount)
   c. Apply global velocity offset from user
   d. Clamp 1-127
5. Resolve timing:
   a. Start with grid position in samples
   b. Apply swing: if step is odd, delay by (swingPercent/100) * stepDuration
   c. Apply per-step timing offset from template (converted from ms to samples)
   d. Apply humanization jitter: random +/- (humanization * 5ms) converted to samples
   e. Apply global timing offset from user
6. Resolve gate length:
   a. Get gate percentage from template (staccato=40%, normal=60%, legato=90%)
   b. Calculate note-off time: noteOn + (stepDuration * gatePercent)
   c. For legato gates that would overlap the next note, extend to next note-on minus 1 sample
7. Resolve articulation:
   a. SLIDE: set note-on time to overlap previous note by glideAmount ms
   b. BEND: schedule pitch bend message at note-on (bend up by bendRange), then pitch bend center message at note-on + bendDuration
   c. No articulation: standard note-on/note-off pair
8. Queue the MIDI event(s) for output
```

**Lock point awareness:**
The lock engine uses lock point metadata to adjust behavior:
- UNISON lock: tighten timing to match kick exactly (reduce humanization jitter to near-zero on this step)
- ALTERNATE lock: ensure bass velocity doesn't exceed the resting drum element's last hit
- ANTICIPATE lock: apply slide articulation automatically if not already present
- FILL lock: reduce velocity by 10-15% and force staccato gate

**MIDI output format:**
Bass notes are output on a user-configured MIDI channel (default: channel 2). When pitch hints are disabled, all notes output on a single configurable root note (default: C2 / note 36). When enabled, the PitchEngine assigns note numbers per step — see the Pitch system section below.

**Bar tracking:**
At each `step == 0` transition the engine computes `absoluteBar = floor(absPPQ / 4)` and `phraseBar = absoluteBar % 8`. If an `ExpandedPhrase` is loaded and pitch is enabled, it calls `PitchEngine::computeBarFromState(phrase.bars[phraseBar], ...)` instead of the single-bar `computeBar()`. `currentPhraseBar` is exposed to the processor for the UI position indicator.

### PhraseExpander

Runs on the message thread only. Takes a seed `GrooveTemplate`, `GenreProfile`, and the user's `density` / `tension` values and produces an `ExpandedPhrase` containing 8 `BarPitchState` objects.

**`BarPitchState`** (per bar):
```
PitchRole stepRoles[16]       — pitch role per step (NONE for inactive steps)
int       stepOctaveOffset[16] — 0=base, 1=+1 octave, -1=-1 octave
float     deviationLevel       — 0.0–1.0 how far this bar strays from seed
```

**Phrase arc** (default deviation per bar):
`{ 0.0, 0.1, 0.2, 0.4, 0.3, 0.5, 0.7, 0.2 }`

Effective deviation = `arc[bar] × profile.maxDeviation × density`.

**Per-bar transformations applied in order:**
1. `computeBarRoles` — walks the available intervals list based on `deviation × tension`; unison (ROOT) and APPROACH steps are immutable
2. `enforceRootGravity` — converts excess non-root steps to ROOT if the bar falls below its minimum root fraction (60% → 70% on bar 8)
3. `applyTurnaround` — sets last active step of bars 4 and 8 (0-indexed: 3 and 7) to b7 (or ROOT at very low tension)
4. `computeOctaveOffsets` — probabilistic +1 octave displacement on non-unison accent/fill steps; probability follows the bar's energy contour (bars 5–7 heaviest, bar 8 = 0)

Genre clamping is applied to density and tension before any computation. Each genre defines `densityClampMin/Max`, `tensionClampMin/Max`, and `maxOctaveDisplPerBar`.

### PitchEngine

Runs on the audio thread, called once per bar boundary from LockEngine.

Two entry points:
- `computeBar(tmpl, profile, params)` — derives pitch roles from the template at runtime (used when phrase expansion is off or when pitch is disabled)
- `computeBarFromState(state, profile, params)` — uses pre-computed roles from `BarPitchState`; applies `state.stepOctaveOffset[step] * 12` to the final MIDI note

Both methods run a two-pass algorithm:
1. **Pass 1** — resolve all non-APPROACH steps: map role → semitone, snap to scale, apply density limiting, accumulate unique pitch count
2. **Pass 2** — resolve APPROACH steps: find the next active step's resolved note and subtract 1 semitone (chromatic) or the nearest scale tone below (diatonic)

### MidiOutputManager

Handles the actual MIDI buffer writing in processBlock.

```
- Maintains a priority queue of scheduled MidiEvents sorted by sample position
- On each processBlock call, flushes all events whose sample position falls within the current block
- Handles note-off cleanup: tracks all active notes, ensures note-offs are always sent even if the plugin is bypassed or template changes mid-phrase
- Implements "panic" button: sends all-notes-off on the output channel
```

## Parameter list

### Groove parameters

| Parameter | Range | Default | Atomic type |
|-----------|-------|---------|-------------|
| Template index | 0–N | 0 | `Atomic<int>` |
| Swing % | 0–100 | 55 | `Atomic<float>` |
| Humanization % | 0–100 | 20 | `Atomic<float>` |
| Global velocity offset | −64–+64 | 0 | `Atomic<float>` |
| Global timing offset ms | −20–+20 | 0 | `Atomic<float>` |
| Gate length scale | 50–150% | 100 | `Atomic<float>` |
| Glide time ms | 10–300 | 100 | `Atomic<float>` |
| Output MIDI channel | 1–16 | 2 | `Atomic<int>` |
| Output root note | 0–127 | 36 (C2) | `Atomic<int>` |
| Pitch bend range (semitones) | 1–12 | 2 | `Atomic<int>` |
| Input mode (live/internal) | 0 or 1 | 1 | `Atomic<int>` |

### Pitch parameters

| Parameter | Range | Default | Atomic type |
|-----------|-------|---------|-------------|
| Pitch hints enabled | 0/1 | 0 | `Atomic<int>` |
| Scale type | 0–5 (ScaleType enum) | 0 (minor pent.) | `Atomic<int>` |
| Pitch density override | 0–5 (0=auto) | 0 | `Atomic<int>` |
| Chromatic approach | 0/1 | 1 | `Atomic<int>` |

### Phrase expansion parameters

| Parameter | Range | Default | Atomic type |
|-----------|-------|---------|-------------|
| Density | 0.0–1.0 | 0.5 | `Atomic<float>` |
| Tension | 0.0–1.0 | 0.5 | `Atomic<float>` |
| Regen mode | 0=fixed / 1=per-loop / 2=manual | 1 | `Atomic<int>` |
| Clamp override | 0/1 | 0 | `Atomic<int>` |

### Read-only (audio → UI)

| Value | Atomic type |
|-------|-------------|
| Current step (0–15) | `Atomic<int>` |
| Current phrase bar (0–7) | `Atomic<int>` |
| Needs regen flag (per-loop) | `Atomic<int>` |
| Phrase params dirty flag | `Atomic<int>` |

## State persistence

Plugin state saved/restored via `getStateInformation` / `setStateInformation`:
- All parameter values
- Currently selected template index
- Any user edits to the current template (serialized as JSON)
- Drum input note mapping

Use `juce::ValueTree` for state management — serialize to XML binary.

## Performance budget

Target: < 1% CPU on a modern machine. Since we're only processing MIDI (no DSP), this should be trivially achievable. The main concern is lock-free correctness, not performance.

Avoid allocations on the audio thread. Pre-allocate all MIDI event buffers. The scheduled event queue should be a fixed-size ring buffer.

## Testing strategy

- **Unit tests** for GrooveTemplate serialization (round-trip JSON)
- **Unit tests** for LockEngine: given a template and a step index, verify the output MIDI events (velocity, timing, gate, articulation)
- **Unit tests** for PatternAnalyzer: given a sequence of MIDI note-ons at known sample positions, verify the resulting DrumState
- **Integration test** for the full processBlock path: feed drum MIDI in, verify bass MIDI out
- **GUI tests** can be manual — verify step sequencer renders correctly, template browser filters work

Use JUCE's `UnitTest` framework or Catch2.
