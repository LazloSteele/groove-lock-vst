# GUI Specification — Groove Lock VST

## Window

- Default size: 780 x 560 px
- Resizable: yes, with aspect ratio lock
- Minimum size: 640 x 480 px
- Background: dark (#0a0a0a)
- All text: monospace font family (system monospace or bundled)

## Layout

```
┌──────────────────────────────────────────────────────────────────────┐
│  HEADER BAR                                                         │
│  Logo / name / preset name / prev-next arrows / save                │
├────────────────────────────────────────┬─────────────────────────────┤
│                                        │                             │
│  MAIN PANEL (left, ~70% width)         │  SIDEBAR (right, ~30%)      │
│                                        │                             │
│  ┌──────────────────────────────────┐  │  ┌───────────────────────┐  │
│  │  DRUM GRID (step sequencer)     │  │  │  TEMPLATE BROWSER     │  │
│  │  3-5 rows, 16 columns           │  │  │  Search box            │  │
│  │  Amber color scheme              │  │  │  Genre filter          │  │
│  └──────────────────────────────────┘  │  │  Region filter         │  │
│                                        │  │  Scrollable list       │  │
│  ┌──────────────────────────────────┐  │  │  with mood tags        │  │
│  │  LOCK POINTS (dot row)          │  │  └───────────────────────┘  │
│  │  16 positions, colored dots      │  │                             │
│  └──────────────────────────────────┘  │  ┌───────────────────────┐  │
│                                        │  │  GLOBAL CONTROLS      │  │
│  ┌──────────────────────────────────┐  │  │  Swing knob            │  │
│  │  BASS GRID (step sequencer)     │  │  │  Humanize knob         │  │
│  │  1-2 rows, 16 columns           │  │  │  Vel offset knob       │  │
│  │  Blue color scheme               │  │  │  Timing offset knob    │  │
│  └──────────────────────────────────┘  │  │  Gate scale knob       │  │
│                                        │  │  Glide time knob       │  │
│  ┌──────────────────────────────────┐  │  └───────────────────────┘  │
│  │  INFO BAR                        │  │                             │
│  │  Groove description, lock detail │  │  ┌───────────────────────┐  │
│  │  when hovered/selected           │  │  │  I/O CONFIG           │  │
│  └──────────────────────────────────┘  │  │  Input mode toggle     │  │
│                                        │  │  Output channel         │  │
│                                        │  │  Root note              │  │
│                                        │  │  Drum mapping button   │  │
│                                        │  │  Panic button           │  │
│                                        │  └───────────────────────┘  │
├────────────────────────────────────────┴─────────────────────────────┤
│  TRANSPORT BAR                                                       │
│  Play/stop (internal) / step position indicator / tempo display      │
└──────────────────────────────────────────────────────────────────────┘
```

## Color system

### Step sequencer cells — drums (amber ramp)
| Velocity tier | Fill color | Border |
|---------------|-----------|--------|
| OFF | transparent (#ffffff04 on beat, #ffffff02 off) | #ffffff0a |
| GHOST | #5a4a3a | #6a5a4a |
| MED | #b87830 | #c88838 |
| FULL | #e89540 | #f0a050 |
| ACCENT | #ff6622 | #ff4400 (2px) |

### Step sequencer cells — bass (blue ramp)
| Velocity tier | Fill color | Border |
|---------------|-----------|--------|
| OFF | transparent | #ffffff0a |
| GHOST | #2a3a4a | #3a4a5a |
| MED | #2868a0 | #3878b0 |
| FULL | #3a90d8 | #4aa0e8 |
| ACCENT | #22bbff | #00ccff (2px) |

### Lock point dots
| Lock type | Color | Meaning |
|-----------|-------|---------|
| Unison | #ff6622 | Kick + bass together |
| Alternate | #4a9eff | One rests, other plays |
| Anticipate | #aa77ff | Bass leads into next hit |
| Fill | #44cc88 | Bass fills empty space |

### Genre accent colors (used for template browser highlights)
| Genre | Accent |
|-------|--------|
| G-Funk | #4a9eff |
| Mobb | #ff5555 |
| Hyphy | #ffaa22 |
| Wonky | #aa77ff |
| Modern West Coast | #44cc88 |

### Timing indicators inside cells
| Timing | Symbol | Display |
|--------|--------|---------|
| grid | (none) | No indicator |
| push | ↑ | Small up arrow, 7px |
| lay | ↓ | Small down arrow, 7px |
| flam | ⫶ | Double bar, 7px |
| drag | ≈ | Wavy, 7px |
| slide | ╲ | Diagonal, 7px |
| bend | ∿ | Curve, 7px |

## Step sequencer component

### Cell behavior
- **Display:** Colored rectangle with optional timing indicator
- **Click:** Cycle through velocity tiers (OFF → GHOST → MED → FULL → ACCENT → OFF)
- **Right-click:** Open context menu to set timing type
- **Hover:** Show tooltip with velocity range and timing offset
- **Beat markers:** Steps 1, 5, 9, 13 (beat boundaries) have slightly brighter grid lines

### Row labels
- Left-aligned, 50px wide
- Monospace, 10px, muted color
- Drum rows: #999
- Bass rows: #6699cc

### Step numbers
- Top of grid, centered above each column
- Beat boundaries (1, 5, 9, 13) brighter than subdivisions
- Monospace, 8px

### Playhead indicator
- Vertical line or column highlight that follows the current step during playback
- Color: white at 20% opacity
- Only visible during playback

## Lock point row

- Positioned between drum grid and bass grid
- 16 positions, aligned with step columns
- Each position is either empty or shows a colored dot (8px diameter)
- Click a dot to expand the lock description in the info bar below
- Active (clicked) dot scales up to 12px with a glow ring

## Template browser

- Search input at top (filters by name, mood, description text)
- Genre filter: horizontal toggle buttons (All / G-Funk / Mobb / Hyphy / Wonky / Modern WC)
- Region filter: dropdown
- Results list: scrollable, each entry shows:
  - Template name (14px, bold, white)
  - Region — tempo range — swing (10px, muted)
  - Mood tag (pill, genre-colored)
- Click to load template into the main view
- Currently selected template highlighted with genre accent color border

## Global controls

Six rotary knobs in a 2x3 grid:
- Each knob: 40px diameter, dark fill, accent-colored indicator arc
- Label below each knob, 9px, muted
- Value display: centered inside knob or directly below label
- Drag vertically to adjust (standard JUCE knob behavior)
- Double-click to reset to default

## Info bar

- Full width below the bass grid
- Shows the groove description text by default
- When a lock point is clicked, shows the lock point description instead
- When a step is hovered, shows the velocity and timing detail for that step
- Text: 11px, line-height 1.5, muted color with accent-colored label prefixes

## Transport bar

- Bottom of window
- Internal play/stop button (for auditing the pattern without DAW transport)
- Step position indicator: 16 small dots, current step highlighted
- Tempo display: shows DAW tempo when synced, or internal tempo when standalone
- Pattern active toggle (on/off — mutes MIDI output when off)

## Interaction patterns

### Loading a template
1. User clicks template in browser
2. Template data loads into staging buffer (UI thread)
3. Atomic flag signals audio thread
4. Audio thread swaps to new template at next bar boundary (or immediately if stopped)
5. GUI updates: drum grid, bass grid, lock points all refresh

### Editing a step
1. User clicks a cell in the drum or bass grid
2. Velocity cycles to next tier
3. Change written to a "user override" layer on top of the template
4. Audio thread reads the override on next step
5. If the user has made any edits, show a "Modified" indicator next to the preset name

### Saving a user preset
1. User clicks save button in header
2. Dialog: save as new preset (user presets directory) or overwrite
3. Serializes current template (with overrides applied) to JSON
4. Writes to user presets directory

## Accessibility

- All knobs support keyboard input (arrow keys for fine, shift+arrow for coarse)
- Step grid supports keyboard navigation (arrow keys to move, enter to toggle, right-click equivalent via shift+enter)
- All interactive elements have JUCE accessibility labels
- High contrast mode: replace semi-transparent backgrounds with solid dark fills
