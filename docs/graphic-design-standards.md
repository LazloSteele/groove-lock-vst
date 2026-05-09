# Plugin UI Design Rules (Bassline Generator VST)

## Core Principle
Design for **clarity, speed, and musical intuition**.  
The UI should feel like an instrument, not a configuration panel.

---

## 1. Layout Rules

### Structure
- Top: **Style selection**
  - Subgenre
  - Groove presets
- Middle: **Core controls**
  - Density
  - Tension
  - Pitch complexity
- Bottom: **Feedback**
  - MIDI / rhythm visualization

### Flow
User workflow must be obvious:
1. Select style  
2. Pick groove  
3. Adjust feel  
4. Done  

---

## 2. Control Design Rules

- Use **large, central controls** for main parameters
- Avoid small or crowded UI elements
- Every control must produce:
  - immediate audible change
  - visible feedback

### Parameter behavior
- Do NOT use fully linear scaling if it produces unusable output
- Implement:
  - soft limits
  - “safe zones” for musical results

Example:
- 0–70% = musical range  
- 70–100% = labeled “experimental”

---

## 3. Complexity Management

- Limit visible options at any time
- Group related controls
- Avoid technical language

### Labeling
Use musical language:
- Good: “Groove”, “Bounce”, “Tension”
- Avoid: technical/internal terminology

---

## 4. Visual Feedback Rules

Every parameter must have visible feedback:

- Density → note count / visual density
- Tension → intensity or variation
- Pitch complexity → spread or variation

### MIDI visualization
- Primary notes → standard color
- Ghost notes → dimmed
- Active playback → bright highlight

---

## 5. Color System (60:30:10 Rule)

### Primary (60%)
- Usage:
  - Backgrounds
  - Panels
- Requirements:
  - Dark, neutral
  - Low saturation
- Goal:
  - Stay visually unobtrusive

---

### Focus (30%)
- Usage:
  - Controls (knobs, sliders, buttons)
  - Selected states
- Requirements:
  - Medium contrast
  - Clearly visible against background
- Goal:
  - Define interactivity

---

### Accent (10%)
- Usage:
  - Highlights
  - Active states
  - Parameter changes
- Requirements:
  - High contrast
  - High saturation
- Goal:
  - Indicate meaning and intensity

---

## 6. Default Color Palette

### Primary
- #121417 (dark charcoal)

### Focus
- #3A4652 (blue-gray)

### Accent
- #FF9F1C (amber/orange)

---

## 7. Color Behavior Rules

- Accent color represents:
  - intensity
  - activity
  - change

### Mapping
- Density → brightness/saturation increase
- Tension → intensity or subtle animation
- Pitch complexity → number of highlighted elements

---

## 8. Ratio Constraints

- Primary: ~60% of screen
- Focus: ~30% of screen
- Accent: ~10% of screen

### Hard limits
- Accent should NOT exceed 15–20%
- Too much accent = visual noise

---

## 9. Interaction Rules

- Changes must feel immediate (<100ms perceived delay)
- Hover/active states must be clearly visible
- Selected options must be obvious at a glance

---

## 10. Usability Rules

- User should get a usable result in <10 seconds
- No hidden behavior
- No ambiguous controls

---

## 11. Anti-Patterns (Do NOT implement)

- Generic “developer UI” (default sliders, no styling)
- Overly complex layouts
- Multiple competing accent colors
- High saturation across entire UI
- Controls with unclear effect
- Overly technical labels

---

## 12. Success Criteria

The UI is successful if:
- A new user can produce a groove quickly without instructions
- Changes are both audible and visible
- The interface feels responsive and intentional
- The plugin appears professional at first glance