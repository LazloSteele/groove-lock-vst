# Groove Lock VST

A MIDI generator for West Coast hip-hop bass. It receives drum MIDI from your DAW, locks a bass rhythm against it using a library of genre-specific groove templates, and outputs bass MIDI on a separate channel. No audio — purely MIDI.

Target genres: G-Funk, Mobb, Hyphy, Wonky, Modern West Coast.

---

## Contents

- [Build](#build)
- [Installation](#installation)
- [DAW setup (Ableton)](#daw-setup-ableton)
- [Signal flow](#signal-flow)
- [Quick start](#quick-start)
- [Template browser](#template-browser)
- [Groove controls](#groove-controls)
- [Pitch system](#pitch-system)
- [8-bar phrase expansion](#8-bar-phrase-expansion)
- [Parameter reference](#parameter-reference)
- [Troubleshooting](#troubleshooting)

---

## Build

Requires CMake 3.22+ and a C++17 compiler. JUCE 7 is fetched automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 bundle is written to `build/GrooveLock_artefacts/Release/VST3/`.  
Presets are copied into the bundle automatically as a post-build step.

---

## Installation

Copy the VST3 bundle to your system plugin folder:

- **Windows:** `C:\Program Files\Common Files\VST3\`
- **macOS:** `~/Library/Audio/Plug-Ins/VST3/`
- **Linux:** `~/.vst3/`

Rescan plugins in your DAW after copying.

---

## DAW setup (Ableton)

Groove Lock acts as an **instrument** that outputs MIDI. Ableton does not allow pure MIDI VST3 effects, so the architecture works around this:

1. **Create a MIDI track.** Load Groove Lock as the instrument on that track.
2. **Set the track's MIDI output** to route to a second MIDI track (the bass track). In Ableton: set "MIDI To" on the Groove Lock track to the bass instrument track, input channel = the output channel you've configured in Groove Lock (default: channel 2).
3. **On the bass instrument track,** set "MIDI From" to the Groove Lock track, and arm it for recording or set "Monitor" to In.
4. **Drum input:** Groove Lock can either use its internal template pattern or follow live drum MIDI. For live input, set the Input Mode toggle to "Live MIDI in" and route drum MIDI to the Groove Lock track.

At playback, the bass instrument track receives the generated MIDI and plays through whatever synth you've loaded there.

---

## Signal flow

```
Drum MIDI In (optional)
        │
        ▼
  Pattern Analyzer          ← classifies kicks, snares, hats from MIDI notes
        │
        ▼
   Lock Engine              ← applies template rhythm, velocity, timing, gate, articulation
        │
        ├── PhraseExpander  ← pre-computes 8-bar pitch roles (Density/Tension driven)
        │
        ▼
   Pitch Engine             ← assigns MIDI note numbers (when pitch hints enabled)
        │
        ▼
  MIDI Output Manager       ← writes to DAW MIDI buffer on the configured channel
```

---

## Quick start

1. Load a template from the browser (top of sidebar).
2. Hit play in your DAW. You should hear the bass rhythm pattern playing.
3. All notes default to a single root note (C2) until you enable pitch hints.
4. To hear melodic content: scroll to the **Pitch** section at the bottom of the sidebar, toggle **Pitch hints** on, set a root note and scale to match your track.
5. To hear the 8-bar phrase evolve: move the **Density/Tension XY pad** away from center.

---

## Template browser

The top of the sidebar contains the template browser.

**Search** filters templates by name, mood, and description text.  
**Genre filter** limits the list to one genre.

Each entry shows the template name, region, tempo range, and a genre-colored tag. Click any template to load it — the drum grid, bass grid, and lock point row all update immediately.

Use the **< >** arrows in the header to step through templates in order.

### Lock point row

The colored dots between the drum and bass grids show the interlocking relationships:

| Color | Type | Meaning |
|-------|------|---------|
| Orange | Unison | Kick and bass hit together |
| Blue | Alternate | One rests while the other plays |
| Purple | Anticipate | Bass leads into the next drum hit with a slide |
| Green | Fill | Bass fills a gap where no core drum element plays |

Click a dot to see its description in the info bar below the bass grid.

---

## Groove controls

Six rotary knobs in the sidebar control the feel of the output:

| Knob | Range | What it does |
|------|-------|-------------|
| **Swing** | 0–100% | Delays every other 16th note. 50% = straight, 65% = hip-hop shuffle |
| **Humanize** | 0–100% | Randomizes velocity and timing within genre-appropriate ranges |
| **Vel Off** | −64–+64 | Global velocity offset applied after all other processing |
| **Timing** | −20–+20 ms | Shifts the entire bassline earlier (negative) or later (positive) |
| **Gate** | 50–150% | Scales all note lengths. Below 100% = tighter/more staccato |
| **Glide** | 10–300 ms | Duration of slide articulations between notes |

**I/O controls** (below the knobs):

- **Live MIDI in** toggle — when on, the plugin reads incoming drum MIDI and attempts to match the template to your live drum pattern. When off, it uses the template's internal drum pattern.
- **Output channel** — which MIDI channel the bass notes are sent on. Default: channel 2.
- **PANIC** button — sends all-notes-off on the output channel. Use if notes get stuck.

---

## Pitch system

By default, all bass hits output on a single configurable root note (no pitch movement). The pitch system, when enabled, assigns different scale degrees to different steps based on each step's lock type and the selected genre profile.

### Enabling pitch

In the **Pitch** section at the bottom of the sidebar:

1. Toggle **Pitch hints** on.
2. Set **Root** to the key your track is in (C through B).
3. Set **Scale** — auto-populated from the template's genre but you can override. Options: Minor Pentatonic, Natural Minor, Dorian, Blues, Phrygian, Chromatic.
4. Use **Oct –** / **Oct +** to set which octave the root lives in. The current note + octave is shown in the center display (e.g. "C2"). C2 (MIDI note 36) is typical for bass.
5. Toggle **Chromatic** on to allow half-step approach tones before target notes. This is where slides from the anticipate lock points get their characteristic "into the note" feel.

### How pitch roles are assigned

Each lock type has a default pitch role:

| Lock type | Default pitch role |
|-----------|-------------------|
| Unison (kick+bass together) | Root — anchors the low end |
| Alternate (call-response) | 5th on strong beats, b7 on weak beats |
| Anticipate (leads into next hit) | Approach tone — half step below the target |
| Fill (decorative) | From the genre's preferred intervals list |

Steps with no lock point inherit from the nearest lock point within 2 steps.

### Density control

The **Density** dot slider (1–5, or 0 = auto from template) limits how many unique pitches appear in a single bar. At 1: root only. At 2: root + one other. At 5: full movement across the genre's preferred intervals. The template has a suggested density; the auto setting uses it. The XY pad's X axis also influences effective density across the 8-bar phrase (see below).

---

## 8-bar phrase expansion

The phrase expander takes the 1-bar seed pattern and generates an 8-bar phrase where the pitch content evolves over time. The rhythm, velocity, timing, and articulation all stay identical to the seed — only the MIDI note assignments change bar to bar.

**This only has an audible effect when Pitch hints is enabled.**

### The XY pad

The 80×80 pad in the sidebar (below the template list) is the primary phrase control. X = Density, Y = Tension.

```
HIGH TENSION
    ↑
    │  ·           ●  ← cursor
    │
    └──────────────→
                 HIGH DENSITY
```

- **Density (X axis, left → right):** How active and busy the phrase gets. At far left, only the root note on the minimum number of steps. At far right, the phrase arc reaches maximum deviation from the seed, ghost fills appear, and pitch variety increases.
- **Tension (Y axis, bottom → top):** How adventurous the pitch content is. At bottom, only root and 5th are available, no slides or bends. At top, the full preferred-intervals list is accessible, chromatic approach tones and blue notes appear at the phrase peak.

**Default center (0.5, 0.5)** = the seed pattern as authored. Identical to what you'd hear with phrase expansion off.

**Double-click** the pad to reset to center.

The slightly brighter rectangle inside the pad shows the genre-clamped accessible range — a Mobb template stays restrained, a Wonky template opens the full range.

The coordinate readout below the pad shows the current values (e.g. `D 0.62  T 0.48`).

### Phrase arc

The 8 bars follow a fixed contour:

| Bar | Deviation | Character |
|-----|-----------|-----------|
| 1 | 0.0 | Seed — exact as authored |
| 2 | 0.1 | Nearly identical, just starting to breathe |
| 3 | 0.2 | New fill-step pitches opening |
| 4 | 0.4 | Midpoint — b7 turnaround on last step |
| 5 | 0.3 | Settling back slightly |
| 6 | 0.5 | Building again |
| 7 | 0.7 | Peak — maximum pitch variety, octave jumps possible |
| 8 | 0.2 | Resolve — root-heavy, b7 on last step into bar 1 |

The actual deviation at each bar = `arc value × genre max deviation × Density`.

### Regen mode

The dropdown below the readout controls when the phrase is recalculated:

- **Per-Loop** (default) — each time the phrase cycles (bar 8 → bar 1), a new variation is generated. Density and tension bound the results, but the specific note choices change each loop. The groove evolves perpetually.
- **Fixed** — generates once when you load a template or move the pad, then loops identically. Good for recording a specific phrase.
- **Manual** — holds until you click the **Regen** button that appears. Good for auditioning options.

Phrase regeneration always happens on the message thread — it never interrupts audio processing.

### 8-bar position indicator

The row of 8 numbered segments at the bottom-left of the transport bar lights the current phrase bar in orange. Only updates while the DAW transport is running.

---

## Parameter reference

### Groove parameters (automatable)

| Parameter | Range | Default |
|-----------|-------|---------|
| Swing % | 0–100 | 55 |
| Humanize % | 0–100 | 20 |
| Velocity offset | −64–+64 | 0 |
| Timing offset | −20–+20 ms | 0 |
| Gate scale | 50–150% | 100 |
| Glide time | 10–300 ms | 100 |
| Output root note | 0–127 (MIDI) | 36 (C2) |
| Pitch hints enabled | on/off | off |
| Density (phrase) | 0.0–1.0 | 0.5 |
| Tension (phrase) | 0.0–1.0 | 0.5 |

### Pitch parameters

| Parameter | Range | Default |
|-----------|-------|---------|
| Root note | C–B | C |
| Scale | Minor Pent / Nat Minor / Dorian / Blues / Phrygian / Chromatic | Minor Pentatonic |
| Pitch density override | 0 (auto) / 1–5 | 0 |
| Chromatic approach | on/off | on |

### Output / I/O

| Parameter | Range | Default |
|-----------|-------|---------|
| Output MIDI channel | 1–16 | 2 |
| Input mode | Live MIDI / Internal | Internal |
| Regen mode | Fixed / Per-Loop / Manual | Per-Loop |
| Pitch bend range | 1–12 semitones | 2 |

All groove parameters are saved with the DAW project. Template index and pitch settings are also persisted.

---

## Troubleshooting

**No MIDI output**
- Check the output channel matches what the bass track is listening on.
- Make sure the DAW transport is running (the plugin only outputs when `isPlaying` is true).
- Click PANIC to clear any stuck notes, then restart transport.

**Notes are stuck / droning**
- Click PANIC.
- If it happens repeatedly, reduce Glide time (very long glide can create overlapping notes that some synths hold).

**Pitch hints not audible / all notes the same**
- "Pitch hints" toggle must be on (bottom of sidebar — resize the window taller if you can't see it).
- Make sure the root note and scale match your track's key.
- The phrase expansion pitch changes only take effect at bar boundaries. Let one full bar play to hear the first note change.

**8-bar phrase sounds identical every loop**
- Make sure Regen mode is set to Per-Loop, not Fixed.
- Density and Tension both need to be above the genre's lower clamp. The brighter region on the XY pad shows the accessible range — if your cursor is in the dark corner, pitch variation is near zero.
- Pitch hints must be enabled for phrase expansion to produce audible results.

**Crash on opening the plugin GUI**
- Update to the latest build — an early version had a null-pointer crash when the editor was first opened. Fixed in commit `5ea39e7`.

**Plugin not appearing in Ableton**
- Groove Lock is registered as an Instrument, not a MIDI Effect. In Ableton's plugin browser look under **Instruments → VST3**, not under MIDI Effects.

**Ableton shows "no device" or plugin won't load in a MIDI track**
- This is expected if you try to load it as a MIDI effect. Load it on an Instrument track instead.
