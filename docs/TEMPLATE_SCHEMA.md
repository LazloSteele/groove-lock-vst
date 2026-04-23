# Template Schema — Groove Lock VST

## File format

Each groove template is a single `.json` file stored in the `presets/` directory, organized by genre subdirectory.

## Schema

```json
{
  "version": 1,
  "meta": {
    "name": "Sunset Strip Cruise",
    "genre": "G-Funk",
    "region": "Los Angeles",
    "mood": "Smooth / laid-back",
    "tempoMin": 90,
    "tempoMax": 96,
    "swingPercent": 55,
    "description": "Classic LA cruise. Long 808 tails. Snare reverb is key..."
  },
  "drums": [
    {
      "label": "Kick",
      "steps": [4, 0, 0, 0, 0, 0, 2, 0, 4, 0, 0, 0, 0, 0, 0, 1],
      "timing": ["grid", "grid", "grid", "grid", "grid", "grid", "lay", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "lay"]
    },
    {
      "label": "Snare",
      "steps": [0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0],
      "timing": ["grid", "grid", "grid", "grid", "lay", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "lay", "grid", "grid", "grid"]
    },
    {
      "label": "CH",
      "steps": [3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2],
      "timing": ["grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid"]
    }
  ],
  "bass": [
    {
      "label": "Bass",
      "steps": [4, 0, 0, 0, 0, 0, 2, 0, 4, 0, 0, 0, 0, 0, 0, 2],
      "timing": ["grid", "grid", "grid", "grid", "grid", "grid", "lay", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "grid", "slide"]
    },
    {
      "label": "Gate",
      "steps": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
      "timing": ["legato", "grid", "grid", "grid", "grid", "grid", "legato", "grid", "legato", "grid", "grid", "grid", "grid", "grid", "grid", "staccato"]
    }
  ],
  "locks": [
    {
      "step": 0,
      "type": "unison",
      "description": "Kick + bass hit together on 1. Bass matches kick timing exactly. Accent velocity on both."
    },
    {
      "step": 6,
      "type": "unison",
      "description": "Both laid back 8-15ms. Bass legato sustains into beat 3."
    },
    {
      "step": 8,
      "type": "unison",
      "description": "Beat 3 unison. Bass accent matches kick."
    },
    {
      "step": 15,
      "type": "anticipate",
      "description": "Bass turnaround. Slide into root. Staccato gate so it doesn't bleed into beat 1."
    }
  ]
}
```

## Field reference

### meta

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | string | yes | Display name |
| genre | string | yes | One of: "G-Funk", "Mobb", "Hyphy", "Wonky", "Modern West Coast" |
| region | string | yes | Geographic origin (e.g., "Los Angeles", "East Oakland", "Glasgow") |
| mood | string | yes | 2-3 word mood descriptor (e.g., "Smooth / laid-back") |
| tempoMin | number | yes | Minimum recommended BPM |
| tempoMax | number | yes | Maximum recommended BPM |
| swingPercent | number | yes | Default swing percentage (0-100) |
| description | string | yes | Freeform text describing the groove's character and production notes |

### drums[]

Array of 1-5 drum rows. Each row:

| Field | Type | Description |
|-------|------|-------------|
| label | string | Display name ("Kick", "Snare", "CH", "OH", "Cowbell", etc.) |
| steps | int[16] | Velocity tier per step. 0=OFF, 1=GHOST, 2=MED, 3=FULL, 4=ACCENT |
| timing | string[16] | Timing type per step. Values: "grid", "push", "lay", "flam", "drag" |

### bass[]

Array of 1-2 bass rows. First row is the velocity/articulation pattern. Second row (optional) is the gate pattern.

| Field | Type | Description |
|-------|------|-------------|
| label | string | "Bass" for the main row, "Gate" for the gate row |
| steps | int[16] | Velocity tier per step (same encoding as drums) |
| timing | string[16] | For Bass row: articulation per step. Values: "grid", "push", "lay", "slide", "bend", "staccato", "legato". For Gate row: gate type per step. Values: "staccato", "legato", "grid" (grid = normal/default gate). |

### locks[]

Array of lock points. Each:

| Field | Type | Description |
|-------|------|-------------|
| step | int | Step index (0-15) |
| type | string | One of: "unison", "alternate", "anticipate", "fill" |
| description | string | Human-readable explanation of what happens at this step |

## Timing type values — reference

### Drum timing types

| Value | Meaning | Typical offset |
|-------|---------|----------------|
| grid | On the grid exactly | 0ms |
| push | Pushed ahead of grid | -8 to -15ms |
| lay | Laid back behind grid | +8 to +20ms |
| flam | Double trigger (two hits ~20ms apart) | 0ms primary, +15-25ms flam |
| drag | Rapid double hit | 0ms primary, +30-50ms drag |

### Bass articulation types

| Value | Meaning | MIDI implementation |
|-------|---------|---------------------|
| grid | No special articulation | Standard note-on/note-off |
| push | Pushed ahead of grid | Note-on offset -8 to -15ms |
| lay | Laid back | Note-on offset +8 to +20ms |
| slide | Glide/portamento into note | Overlapping note-on with previous note by glideTime ms |
| bend | Pitch bend on attack | Pitch bend message at note-on, resolve to center over 50-100ms |
| staccato | Short gate | Gate = 30-50% of step duration |
| legato | Long gate | Gate = 80-100% of step duration |

## Velocity tier values

| Int value | Tier name | MIDI velocity range |
|-----------|-----------|---------------------|
| 0 | OFF | 0 (no note) |
| 1 | GHOST | 35-55 |
| 2 | MED | 70-90 |
| 3 | FULL | 100-115 |
| 4 | ACCENT | 120-127 |

## Validation rules

- `drums` array must have 1-5 rows, each with exactly 16 steps and 16 timing values
- `bass` array must have 1-2 rows, each with exactly 16 steps and 16 timing values
- All step values must be integers 0-4
- All timing values must be from the valid set for their row type
- Lock point steps must be 0-15 and must reference steps where at least one drum or bass row has a non-zero velocity
- Lock point types must be one of the four valid strings
- `tempoMin` must be <= `tempoMax`
- `swingPercent` must be 0-100
