# Architecture — Groove Lock VST

## Overview

Groove Lock is a MIDI effect plugin. It receives drum MIDI, analyzes it against an internal 16-step grid, applies interlocking groove rules from a template, and outputs bass MIDI. No audio is processed.

## Threading model

JUCE plugins have two threads that matter:

- **Audio thread** (`processBlock`): receives and sends MIDI. Must be lock-free, no allocations, no blocking. All MIDI processing happens here.
- **Message thread** (GUI): handles UI rendering and user interaction. Can allocate, block, etc.

Communication between threads uses lock-free structures only:
- Audio → UI: `juce::AbstractFifo` ring buffer for step position / activity indicators
- UI → Audio: `juce::Atomic<int>` for template selection index, `juce::Atomic<float>` for continuous parameters (swing, humanization), `juce::Atomic<bool>` for triggers (reset pattern, etc.)

For template changes (larger data), use a double-buffer pattern: UI writes the new template to a staging buffer, sets an atomic flag, audio thread swaps pointers on the next block boundary.

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

The template does NOT contain pitch information. The bass pattern defines WHEN to play (which steps), HOW HARD (velocity tier), HOW LONG (gate percentage), and WITH WHAT ARTICULATION (slide, bend, staccato, legato). The user's bass synth and their own MIDI input/scale selection handle pitch.

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
Bass notes are output on a user-configured MIDI channel (default: channel 2). The note NUMBER is not determined by this plugin — the user sets a fixed root note or routes through a scale quantizer. The plugin outputs all bass hits on a single configurable note (default: C2 / note 36). If the user wants melodic content, they process downstream or edit the MIDI after recording.

Alternatively, the plugin can operate in "pitch hint" mode where lock points with pitch suggestions output on different note numbers, but this is a stretch goal, not MVP.

### MidiOutputManager

Handles the actual MIDI buffer writing in processBlock.

```
- Maintains a priority queue of scheduled MidiEvents sorted by sample position
- On each processBlock call, flushes all events whose sample position falls within the current block
- Handles note-off cleanup: tracks all active notes, ensures note-offs are always sent even if the plugin is bypassed or template changes mid-phrase
- Implements "panic" button: sends all-notes-off on the output channel
```

## Parameter list

| Parameter | Range | Default | Automatable | Thread-safe type |
|-----------|-------|---------|-------------|-----------------|
| Template index | 0 - N | 0 | Yes | Atomic<int> |
| Swing % | 0 - 100 | 55 | Yes | Atomic<float> |
| Humanization % | 0 - 100 | 20 | Yes | Atomic<float> |
| Global velocity offset | -64 to +64 | 0 | Yes | Atomic<float> |
| Global timing offset ms | -20 to +20 | 0 | Yes | Atomic<float> |
| Gate length scale | 50 - 150% | 100 | Yes | Atomic<float> |
| Output MIDI channel | 1 - 16 | 2 | No | Atomic<int> |
| Output root note | 0 - 127 | 36 (C2) | Yes | Atomic<int> |
| Pitch bend range (semitones) | 1 - 12 | 2 | No | Atomic<int> |
| Glide time ms | 10 - 300 | 100 | Yes | Atomic<float> |
| Input mode (live/internal) | 0 or 1 | 1 | No | Atomic<int> |
| Pattern active (on/off) | bool | true | Yes | Atomic<int> |

All automatable parameters should be registered as `juce::AudioProcessorParameter` subtypes so the DAW can automate them.

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
