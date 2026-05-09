# Adaptive Step Movement — Implementation Prompt

## What this is

An extension to Groove Lock's LockEngine that moves bass notes to track live drum input. The current system gates bass steps (fire/don't fire). This system repositions them (fire on step 10 instead of step 9 because that's where the kick actually landed).

## The latency contract

The system uses a **one-bar buffer**. It records one bar of incoming drum MIDI, analyzes the pattern, computes bass step movements, then outputs the adapted bass over the NEXT bar while simultaneously recording the next bar of drums. First bar of playback: no drum data yet, output the seed pattern unmodified. Every bar after that: bass reacts to the PREVIOUS bar's drums. This means bass adaptation is always one bar behind the live drums.

This is musically acceptable because:
- Drum patterns in hip-hop are highly repetitive — bar N is usually identical to bar N-1
- The one-bar delay is imperceptible to the user; they hear adapted bass starting from bar 2
- If the drummer changes the pattern, the bass catches up one bar later, which sounds like a bassist who heard the change and adjusted on the next pass — exactly how a real rhythm section works

## Implementation

### Buffer management

In `PluginProcessor::processBlock`:

```
if (playhead crossed bar boundary):
    swap analysisBuffer and outputBuffer
    analysisBuffer.clear()
    adaptedPattern = LockEngine::computeAdaptation(outputBuffer, currentTemplate)
```

`analysisBuffer` (DrumState[16]) collects hits for the bar currently being heard. `outputBuffer` holds the completed previous bar that the LockEngine uses for adaptation. Double-buffer swap at every bar boundary. No allocation on the audio thread — both buffers are pre-allocated.

### LockEngine::computeAdaptation

Called once per bar boundary on the audio thread. Must complete in under 100μs (it's just array math, no allocation). Takes the recorded DrumState and the groove template, returns an `AdaptedPattern` — a modified copy of the seed's bass steps with adjusted positions.

#### Step 1: Build a kick map and snare map from the recorded bar

```
kickMap[16]: velocity at each step (0 if no kick)
snareMap[16]: velocity at each step (0 if no snare)
```

#### Step 2: For each bass step in the seed, compute movement

Process in lock-type priority order: unison first, then anticipate, then alternate. Fills don't move.

**Unison steps — find the nearest kick:**
```
For each seed step with lock type UNISON:
    seedPos = original step position
    Find the kickMap entry nearest to seedPos within ±2 steps
    If found:
        targetPos = that kick's step
        If |targetPos - seedPos| <= 2:
            Move bass to targetPos
        Else:
            Don't move, fire on seedPos (fall back to gate check)
    If no kick within ±2:
        Mute this step (no unison partner)
```

**Anticipate steps — one step before the next kick:**
```
For each seed step with lock type ANTICIPATE:
    Find the next UNISON step's resolved position (after unison movement above)
    targetPos = unisonPos - 1
    If targetPos >= 0 and targetPos is not occupied by another adapted note:
        Move to targetPos
    Else:
        Keep original position
```

**Alternate steps — find the nearest gap:**
```
For each seed step with lock type ALTERNATE:
    seedPos = original step position
    beatStart = (seedPos / 4) * 4  // which beat this step belongs to
    beatEnd = beatStart + 3
    Find the emptiest step in [beatStart, beatEnd] where:
        - kickMap[step] == 0
        - snareMap[step] == 0
        - No other adapted bass note is already placed here
        - |step - seedPos| <= 1
    If found: move there
    Else: keep original position, apply gate check (mute if drums present)
```

**Fill steps — don't move:**
```
    Keep original position
    If kickMap[seedPos] > 0 or snareMap[seedPos] > 0:
        Mute (don't compete with a drum hit that wasn't expected)
    Else:
        Fire as normal
```

#### Step 3: Phrase contour check

After all movements are resolved:
```
For each pair of adjacent adapted notes:
    If they are now < 2 steps apart AND were >= 3 steps apart in the seed:
        Cancel the movement of the lower-priority note (fill > alternate > anticipate > unison)
        Return it to its seed position

For each beat (steps 0-3, 4-7, 8-11, 12-15):
    If the beat had a bass note in the seed but has none after adaptation:
        Either cancel the movement that vacated it, OR
        Insert a ghost fill (vel tier 1, staccato gate) on the vacated step
```

#### Step 4: Update pitch roles

After step positions are finalized:
```
For each moved step:
    Re-evaluate its pitch role based on its NEW position:
        - If it landed on the same step as a kick: role = root (unison behavior)
        - If it's now 1 step before a kick: role = approach
        - If it's in a gap with no adjacent drums: keep the seed's role
    Pass updated roles to PitchEngine
```

### Output

`AdaptedPattern` is a 16-element array of:
```cpp
struct AdaptedStep {
    int position;           // 0-15 (may differ from seed)
    int velocityTier;       // 0-4 (from seed, possibly scaled by kick tracking)
    int pitchRole;          // re-evaluated after movement
    int articulation;       // from seed (anticipate forces slide if step moved)
    int gate;               // from seed
    bool muted;             // true if gate check failed
    int octaveOffset;       // from PhraseExpander
    float kickVelTracking;  // kick velocity at this step for unison scaling (0.0 if N/A)
};
```

The LockEngine reads from `AdaptedPattern` instead of directly from the seed during playback. On each step, it checks `adaptedSteps[currentStep]` — if an adapted note exists at this position and isn't muted, it fires with the specified properties.

### Movement limits (hardcoded)

```cpp
static constexpr int UNISON_MAX_MOVE = 2;       // steps
static constexpr int ANTICIPATE_MAX_MOVE = 2;    // steps
static constexpr int ALTERNATE_MAX_MOVE = 1;     // steps
static constexpr int FILL_MAX_MOVE = 0;          // steps — fills never move
static constexpr int MIN_NOTE_SPACING = 2;       // steps — contour check minimum
static constexpr bool UNISON_CAN_CROSS_BEAT = true;
static constexpr bool OTHER_CAN_CROSS_BEAT = false;
```

### Edge cases

- **First bar of playback:** No drum data. Output seed pattern unmodified. Set a `firstBarFlag` that clears after the first bar boundary.
- **Drummer stops playing:** If the recorded bar has zero kick hits, fall back to seed pattern (assume internal pattern mode temporarily).
- **Drummer plays completely differently from template:** The ±2 step limit prevents catastrophic misalignment. At worst, some unison notes mute because no kick is within range. The fill notes keep playing regardless, maintaining rhythmic continuity.
- **Tempo change mid-bar:** DrumState step positions are recalculated from the playhead on every hit, so they're always correct relative to the current tempo. The adaptation computed at bar boundary uses whatever steps were recorded — tempo changes within the recorded bar are already baked in.
- **Note collisions after movement:** Two bass notes can't occupy the same step. If movement would create a collision, the later-processed note's movement is cancelled. Process priority: unison → anticipate → alternate → fill.

### What NOT to do

- Don't attempt to predict the next bar's drums. One bar of lookback is the model.
- Don't interpolate between the seed pattern and adapted pattern. It's one or the other per bar — no blending.
- Don't move notes during the bar. Adaptation is computed at bar boundaries only. Within a bar, step positions are fixed.
- Don't move notes more than 2 steps. Larger moves break phrase contour regardless of musical justification.
- Don't adapt fill notes. Their function is decorative and position-independent.
