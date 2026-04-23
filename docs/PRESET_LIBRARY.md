# Preset Library — Groove Lock VST

All groove templates as structured data. Each entry maps directly to the TEMPLATE_SCHEMA.md format. Convert each to a `.json` file in the appropriate `presets/{genre}/` subdirectory.

Velocity encoding: 0=OFF, 1=GHOST, 2=MED, 3=FULL, 4=ACCENT
Drum timing: grid, push, lay, flam, drag
Bass timing/articulation: grid, push, lay, slide, bend, staccato, legato
Lock types: unison, alternate, anticipate, fill

---

## 1. Sunset Strip Cruise

- **Genre:** G-Funk | **Region:** Los Angeles | **Mood:** Smooth / laid-back
- **Tempo:** 90–96 | **Swing:** 55%
- **File:** `presets/gfunk/sunset_strip_cruise.json`
- **Description:** Classic LA cruising pattern. Long 808 kick tails. Snare reverb is key. Hats are a steady pulse, open hat lifts before the snare. Everything behind the grid slightly.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| Kick time | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | lay |
| Snare vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Snare time | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid |
| CH vel | 3 | 2 | 3 | 2 | 3 | 2 | 3 | 2 | 3 | 2 | 3 | 2 | 3 | 2 | 3 | 2 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 2 |
| Bass art | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | slide |
| Gate | legato | grid | grid | grid | grid | grid | legato | grid | legato | grid | grid | grid | grid | grid | grid | staccato |

**Locks:**
- Step 0: unison — Kick + bass hit together on 1. Bass matches kick timing exactly. Accent velocity on both.
- Step 6: unison — Both laid back 8–15ms. Bass legato sustains into beat 3.
- Step 8: unison — Beat 3 unison. Bass accent matches kick.
- Step 15: anticipate — Bass turnaround. Slide into root. Staccato gate so it doesn't bleed into beat 1.

---

## 2. Compton Bounce

- **Genre:** G-Funk | **Region:** Los Angeles | **Mood:** Bouncy / party
- **Tempo:** 94–100 | **Swing:** 60%
- **File:** `presets/gfunk/compton_bounce.json`
- **Description:** More active than Sunset Strip. Double kick before beat 3 creates the bounce. Clap layered with snare. Cowbell on the 'e' of 2 every other bar.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 3 | 2 | 4 | 0 | 0 | 0 | 0 | 2 | 0 | 0 |
| Kick time | grid | grid | grid | grid | grid | grid | grid | push | grid | grid | grid | grid | grid | lay | grid | grid |
| Clap vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Clap time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| Rim vel | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 1 | 0 | 0 | 1 | 0 |
| Rim time | grid | grid | lay | grid | grid | grid | grid | lay | grid | grid | grid | lay | grid | grid | lay | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 1 | 0 | 0 | 0 | 3 | 2 | 4 | 0 | 0 | 1 | 0 | 2 | 1 | 0 |
| Bass art | grid | grid | lay | grid | grid | grid | grid | push | grid | grid | grid | lay | grid | lay | lay | grid |
| Gate | legato | grid | staccato | grid | grid | grid | staccato | staccato | legato | grid | grid | staccato | grid | legato | staccato | grid |

**Locks:**
- Step 0: unison — Beat 1 lock. Both accent, both on grid.
- Step 2: alternate — Bass ghost mirrors rim ghost. Both laid back. Staccato gate.
- Step 6: unison — Bounce moment — bass octave jump matches double kick exactly.
- Step 7: unison — Second bounce hit. Both pushed early for pickup energy.
- Step 13: alternate — Bass fills where kick hits. Both laid back. Legato gate sustains through.

---

## 3. East Oakland Creep

- **Genre:** Mobb | **Region:** East Oakland | **Mood:** Dark / menacing
- **Tempo:** 80–86 | **Swing:** 0%
- **File:** `presets/mobb/east_oakland_creep.json`
- **Description:** Half-time. Snare on 3 only. Long 808 kick with max sustain. Cold, mechanical, zero swing. Every hit is deliberate. Stark and threatening.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Kick time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| Snare vel | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Snare time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| CH vel | 3 | 0 | 3 | 0 | 3 | 0 | 3 | 0 | 3 | 0 | 3 | 0 | 3 | 0 | 3 | 0 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Bass art | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| Gate | legato | grid | grid | grid | grid | grid | grid | grid | legato | grid | grid | grid | grid | grid | grid | grid |

**Locks:**
- Step 0: unison — Beat 1. Both accent. Bass sustains through the entire half. Sidechain the bass under kick.
- Step 8: unison — Beat 3 with snare. Triple lock — kick, snare, bass all hit. Maximum impact.

---

## 4. Richmond Sideshow

- **Genre:** Mobb | **Region:** Richmond, CA | **Mood:** Aggressive / heavy
- **Tempo:** 78–84 | **Swing:** 50%
- **File:** `presets/mobb/richmond_sideshow.json`
- **Description:** Heavier Richmond variant. Kick doubles on 'and' of 3. Barely perceptible swing. Snare flam. Tom fill once every 4 bars.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| Kick time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid |
| Snare vel | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Snare time | grid | grid | grid | grid | grid | grid | grid | grid | flam | grid | grid | grid | grid | grid | grid | grid |
| Tom vel | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 2 | 1 |
| Tom time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | lay |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 4 | 0 | 2 | 0 | 0 | 0 | 0 | 0 |
| Bass art | grid | grid | grid | grid | grid | grid | grid | slide | grid | grid | lay | grid | grid | grid | grid | grid |
| Gate | legato | grid | grid | grid | grid | grid | grid | staccato | legato | grid | staccato | grid | grid | grid | grid | grid |

**Locks:**
- Step 0: unison — Beat 1 lock. Both on grid, both accent.
- Step 7: anticipate — Bass chromatic approach — slide into beat 3. Ghost velocity, staccato.
- Step 8: unison — Beat 3 triple impact: kick + snare flam + bass. Massive.
- Step 10: unison — Kick double = bass octave drop. Both laid back.
- Step 13: fill — Bass drops out. Tom fill speaks alone. Re-enter hard on next bar.

---

## 5. Ghost Ride

- **Genre:** Hyphy | **Region:** East Oakland | **Mood:** Bouncy / energetic
- **Tempo:** 105–112 | **Swing:** 62%
- **File:** `presets/hyphy/ghost_ride.json`
- **Description:** Classic hyphy bounce. Kick is active — 5 hits per bar. 32nd hat roll on beat 4. Cowbell on 'e' of 2 is the signature. Everything pushes forward.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 2 | 0 | 0 | 2 | 0 | 4 | 0 | 0 | 0 | 0 | 2 | 0 | 1 |
| Kick time | grid | grid | grid | push | grid | grid | grid | grid | grid | grid | grid | grid | grid | push | grid | lay |
| Snare vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Snare time | grid | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | drag | grid | grid | grid |
| CH vel | 3 | 1 | 3 | 1 | 3 | 1 | 3 | 1 | 3 | 1 | 3 | 1 | 3 | 3 | 3 | 3 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | push | push | push | push |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 2 | 2 | 0 | 1 | 2 | 1 | 4 | 0 | 1 | 0 | 0 | 2 | 1 | 1 |
| Bass art | grid | grid | staccato | push | grid | staccato | push | staccato | grid | grid | staccato | grid | grid | push | staccato | lay |
| Gate | legato | grid | staccato | staccato | grid | staccato | staccato | staccato | legato | grid | staccato | grid | grid | staccato | staccato | staccato |

**Locks:**
- Step 0: unison — Beat 1 lock. Accent, legato gate.
- Step 2: fill — Bass fill between kicks. Staccato, medium velocity.
- Step 3: unison — Both pushed early. Bass matches kick's anticipation.
- Step 5: fill — Bass ghost fill. Staccato. 16th bounce without clutter.
- Step 6: anticipate — Bass pushes early to anticipate beat 3. Kick stays on grid. Tension IS the groove.
- Step 13: unison — Both pushed. Bass staccato under 32nd hat roll.

---

## 6. Glasgow Glitch

- **Genre:** Wonky | **Region:** Glasgow | **Mood:** Off-kilter / cerebral
- **Tempo:** 125–135 | **Swing:** 72%
- **File:** `presets/wonky/glasgow_glitch.json`
- **Description:** HudMo / Rustie territory. Deep swing makes the grid elastic. Kick avoids obvious downbeats. CR-78 hats in uneven groupings. Found sound spring boing as signature.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 2 | 0 | 0 | 4 | 0 |
| Kick time | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | push | grid | grid | lay | grid |
| Snare vel | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 |
| Snare time | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| CR78 vel | 3 | 0 | 2 | 0 | 3 | 0 | 2 | 0 | 3 | 0 | 2 | 0 | 3 | 0 | 2 | 0 |
| CR78 time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 2 | 0 | 0 | 0 | 3 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 3 | 0 | 0 | 0 |
| Bass art | slide | grid | grid | grid | slide | grid | grid | grid | slide | grid | grid | grid | slide | grid | grid | grid |
| Gate | legato | grid | grid | grid | legato | grid | grid | grid | legato | grid | grid | grid | legato | grid | grid | grid |

**Locks:**
- Step 0: alternate — Bass on 1 while kick avoids it. Slide articulation.
- Step 2: alternate — Kick enters after bass sustains. They never collide.
- Step 4: alternate — Bass accent on the 'and' of 1. Kick silent. Displacement IS the groove.
- Step 7: alternate — Kick full hit. Bass resting. Clean separation.
- Step 12: alternate — Bass accent, snare medium. EQ separation critical.

---

## 7. Aqua Crunk

- **Genre:** Wonky | **Region:** Glasgow | **Mood:** Heavy / maximalist
- **Tempo:** 130–140 | **Swing:** 70%
- **File:** `presets/wonky/aqua_crunk.json`
- **Description:** Heavier Glasgow variant. 808 kick pitched low with distortion. Clap = layered clap + noise burst. Everything swings hard. Controlled madness.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 2 |
| Kick time | grid | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | push |
| Clap vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 |
| Clap time | grid | grid | grid | grid | flam | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| CH vel | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 1 | 0 | 0 | 2 | 0 | 1 | 0 | 2 | 0 | 3 | 1 | 0 | 2 | 0 | 0 | 3 |
| Bass art | grid | grid | grid | slide | grid | staccato | grid | lay | grid | slide | grid | grid | bend | grid | grid | push |
| Gate | staccato | grid | grid | legato | grid | staccato | grid | legato | grid | legato | staccato | grid | legato | grid | grid | staccato |

**Locks:**
- Step 0: unison — Kick accent + bass ghost. Velocity inversion — kick dominates, bass provides sub bed.
- Step 3: fill — Bass slide into step 4's clap zone. Medium vel, legato.
- Step 7: unison — Kick full + bass medium. Kick leads, bass supports laid back.
- Step 9: fill — Bass accent solo. No kick. Bass owns the sub here, slide in.
- Step 15: anticipate — Bass pushes early while kick pushes early. Both anticipate beat 1. Staccato bass.

---

## 8. Modern LA Hybrid

- **Genre:** Modern West Coast | **Region:** Los Angeles | **Mood:** Polished / versatile
- **Tempo:** 94–100 | **Swing:** 58%
- **File:** `presets/modern_westcoast/modern_la_hybrid.json`
- **Description:** Contemporary hybrid. G-funk spacing with modern layering. 808 kick + CR-78 click layer. Clap + found sound slap on backbeat. Moderate swing. Clean but characterful.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| Kick time | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | lay |
| Clap vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Clap time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |
| CH vel | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 0 | 0 | 2 | 1 | 2 | 0 | 4 | 0 | 0 | 1 | 0 | 2 | 0 | 1 |
| Bass art | grid | grid | grid | grid | legato | slide | lay | grid | grid | grid | grid | slide | grid | legato | grid | slide |
| Gate | legato | grid | grid | grid | legato | staccato | legato | grid | legato | grid | grid | staccato | grid | legato | grid | staccato |

**Locks:**
- Step 0: unison — Clean beat 1 lock. Both accent, both on grid.
- Step 4: alternate — Bass sustains under clap. Sidechain ducks bass 3–4dB. Legato gate.
- Step 5: anticipate — Bass ghost approach — slide into the kick on step 7. Staccato.
- Step 6: unison — Kick + bass. Both laid back. Legato bass.
- Step 11: anticipate — Chromatic approach slide before snare on step 13.
- Step 15: anticipate — Turnaround slide. Ghost velocity, staccato. Sets up next bar.

---

## 9. Bay Area Slide

- **Genre:** Modern West Coast | **Region:** East Oakland | **Mood:** Smooth / head-nod
- **Tempo:** 88–96 | **Swing:** 60%
- **File:** `presets/modern_westcoast/bay_area_slide.json`
- **Description:** Oakland-inflected modern. Deeper swing than LA. Kick borrows mobb weight but stays lighter. Autovari64 hat texture layer for organic warmth. Everything grooves.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 2 | 0 |
| Kick time | grid | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | grid | lay | grid |
| Clap vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Clap time | grid | grid | grid | grid | lay | grid | grid | grid | grid | grid | grid | grid | lay | grid | grid | grid |
| CH vel | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 | 3 | 1 | 2 | 1 |
| CH time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 4 | 0 | 0 | 0 | 0 | 2 | 2 | 0 | 4 | 0 | 0 | 2 | 0 | 0 | 2 | 0 |
| Bass art | lay | grid | grid | grid | grid | lay | bend | grid | lay | grid | grid | bend | grid | grid | lay | grid |
| Gate | legato | grid | grid | grid | grid | legato | legato | grid | legato | grid | grid | legato | grid | grid | legato | grid |

**Locks:**
- Step 0: unison — Both laid back. Accent. The shared lateness IS the Oakland slide.
- Step 5: unison — Kick + bass together. Both laid back. Bass legato.
- Step 6: fill — Bass melodic fill. Pitch bend. No kick. Expression moment.
- Step 8: unison — Beat 3 lock. Both laid back.
- Step 11: fill — Bass fill with pitch bend. Legato into beat 4's kick.
- Step 14: unison — Both laid back. Bass legato sustains to bar end.

---

## 10. Turf Dance

- **Genre:** Hyphy | **Region:** East Oakland | **Mood:** Percussive / physical
- **Tempo:** 108–115 | **Swing:** 65%
- **File:** `presets/hyphy/turf_dance.json`
- **Description:** Built for movement. Toms create second rhythmic layer. Kick and clap alternate in call-response. Heavier swing. Clap flam on beat 4 for emphasis.

**Drums:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Kick vel | 4 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 4 | 0 | 0 | 2 | 0 | 0 | 1 | 0 |
| Kick time | grid | grid | grid | grid | grid | push | grid | grid | grid | grid | grid | push | grid | grid | lay | grid |
| Clap vel | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 0 |
| Clap time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | flam | grid | grid | grid |
| Tom vel | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 0 | 2 | 0 |
| Tom time | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid | grid |

**Bass:**

| Row | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 |
|-----|---|---|---|---|---|---|---|---|---|----|----|----|----|----|----|-----|
| Bass vel | 0 | 2 | 0 | 0 | 0 | 0 | 2 | 0 | 0 | 2 | 0 | 0 | 0 | 0 | 0 | 0 |
| Bass art | grid | bend | grid | grid | grid | grid | bend | grid | grid | bend | grid | grid | grid | grid | grid | grid |
| Gate | grid | legato | grid | grid | grid | grid | legato | grid | grid | legato | grid | grid | grid | grid | grid | grid |

**Locks:**
- Step 0: alternate — Kick hits, bass silent. The space IS the pocket.
- Step 1: fill — Bass answers with pitch bend. Legato. The call-response engine.
- Step 5: alternate — Kick hits pushed early. Bass waits.
- Step 6: fill — Bass answer. Pitch bend. Legato sustain.
- Step 10: fill — Tom hit — bass drops out. Tom + bass freq separation crucial.

---

## Additional templates to implement

The remaining 8 templates from the drum pattern library (San Quinn Fog, Vallejo Lace, Dumb Stupid, E-40 Bounce, Parliament Freeway, Inglewood Lowrider, LuckyMe Stomp, Bruk Funk) should also have bass groove patterns created following the same structure. Use the genre rules from GENRE_RULES.md to guide the bass pattern design for each. The lock point strategy should be:

- **San Quinn Fog (Mobb/SF):** Bass follows the ride cymbal rhythm rather than hats. Very sparse, 2-3 bass hits per bar. Legato gates, everything laid back.
- **Vallejo Lace (Mobb/Vallejo):** Bass interacts with the clave pattern. Unison on clave hits, rest on kick. Very long sustained notes.
- **Dumb Stupid (Hyphy/Richmond):** Aggressive bass, matches kick activity. 5-6 bass hits per bar, all pushed early. Staccato dominates.
- **E-40 Bounce (Hyphy/Vallejo):** Laid-back bass, everything behind grid. Bass doubles kick but at lower velocity. Legato gates.
- **Parliament Freeway (G-Funk/LA):** Most expressive bass pattern. Slides, bends, ghost fills. Mimics Bootsy Collins' playing style. 6-7 bass hits per bar.
- **Inglewood Lowrider (G-Funk/LA):** Ultra-sparse. 2 bass hits per bar matching the 2 kick hits. Maximum sustain. No articulation.
- **LuckyMe Stomp (Wonky/Glasgow):** Bass interacts with the stomp found sound. Velocity inversion with kick. Slides on every note.
- **Bruk Funk (Wonky/London-Glasgow):** Broken beat bass pattern. Notes cluster in groups of 2-3 then rest. Mix of staccato and legato within clusters.
