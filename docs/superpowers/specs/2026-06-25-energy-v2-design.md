# Energy Watchface v2.0 — Design Spec

## Summary

Redesign the Energy watchface from a plain single-color pillar into a segmented,
configurable energy meter with always-visible time, tap-to-reveal details, and
phone-side config for custom wake/sleep schedules.

---

## Visual Design: Segmented Pillar

- **8 segments** stacked vertically inside a rounded border
- Segments fill from bottom to top; empty from top as the day progresses
- Color per segment based on position in the stack:
  - Top 3 segments: green
  - Middle 3 segments: yellow
  - Bottom 2 segments: red
- Empty segments render as dark/hollow; colored segments represent energy remaining
- **Pulse animation**: topmost filled segment subtly brightens/dims on a ~2s cycle
  ("breathing" alive feel)
- Pulse timer runs at 100ms, **pauses when screen is off** via `app_focus_service`
- B&W platforms (aplite, diorite): white for filled, black for empty via
  `PBL_IF_COLOR_ELSE()`

## Always-Visible Time

- Small, subtle time text below the pillar (GOTHIC_24 or similar)
- White color, understated — the pillar is the visual focus
- Respects 12h/24h system setting

## Tap-to-Reveal (Hybrid)

- Shake or tap shows extra details for 3 seconds:
  - **Percentage** above the pillar (e.g. "73%")
  - **Date** below the time (e.g. "Wed 25 Jun")
  - **Watch battery** as small text (e.g. "BAT 85%")
- Repeated taps reset the 3s timer (don't stack multiple timers)
- Details fade out after 3 seconds, returning to pillar + subtle time

## Energy Cycle

- Energy drains from 1.0 (full) at wake time to 0.0 (empty) at sleep time
- Linear drain based on current time within the wake–sleep window
- Before wake time: clamped to 1.0 (full)
- After sleep time: clamped to 0.0 (empty)
- Segment count = `round(energy_level * 8)`
- Updates on minute tick (no second-level precision needed)

## Config Page (Phone)

### Presets

| Preset | Wake | Sleep |
|--------|------|-------|
| Early Bird | 05:00 | 21:00 |
| Standard | 07:00 | 23:00 |
| Night Owl | 10:00 | 02:00 |

### Custom

- Hour-only pickers for wake and sleep time
- Selecting a preset populates the pickers; editing pickers switches to "Custom"

### Technical

- Config page served via `PebbleKit JS` (inline HTML in `index.js` or separate
  `config.html`)
- Settings sent to watch via `AppMessage` with keys: `wake_hour`, `sleep_hour`
- Watch persists settings via `persist_write()` (survives reboot)
- First install defaults to Standard preset (07:00–23:00) if no settings saved

---

## Architecture

### Files

| File | Purpose |
|------|---------|
| `src/c/main.c` | Window, events, timers, AppMessage handling, focus service |
| `src/c/energy_logic.c` | Pure logic: segment calc, color mapping, time formatting |
| `src/c/energy_logic.h` | State struct, constants, function declarations |
| `src/pkjs/index.js` | Config page launcher, settings relay to watch |

### State Struct

```c
#define NUM_SEGMENTS 8

typedef struct {
    float energy_level;                  // 1.0 (full) to 0.0 (empty)
    int filled_segments;                 // 8 to 0
    int wake_hour;                       // default 7
    int sleep_hour;                      // default 23
    bool show_details;                   // tap reveal active
    char time_str[8];                    // "10:55" or "10:55"
    char date_str[16];                   // "Wed 25 Jun"
    int battery_percent;                 // actual watch battery
    int pulse_anim_frame;                // animation counter
    GColor segment_colors[NUM_SEGMENTS]; // per-segment color
} EnergyState;
```

### Data Flow

```
Phone config page
    -> PebbleKit JS (index.js)
    -> AppMessage {wake_hour, sleep_hour}
    -> persist_write() on watch
    -> energy_logic reads persisted values

Watch events (minute tick, tap, focus)
    -> energy_logic_update() computes segments, colors, strings
    -> energy_logic_draw() renders to screen
```

### Key Behaviors

- Minute tick updates energy level and segment count
- Pulse timer at 100ms, focus-aware (pauses when screen off)
- Tap triggers 3s reveal, repeated taps reset the timer
- Config defaults to Standard (07:00–23:00) if nothing persisted
- AppMessage failure: keep last persisted settings, don't reset
- Night Owl wrap-around: handled when sleep_hour < wake_hour

---

## Edge Cases

- **Night Owl midnight wrap**: sleep_hour (02:00) < wake_hour (10:00) — energy
  calculation crosses midnight correctly
- **Before wake / after sleep**: clamp to full (1.0) / empty (0.0)
- **No config saved**: default to Standard preset
- **AppMessage failure**: retain last persisted settings
- **Rapid taps**: cancel and re-register the 3s reveal timer
- **B&W platforms**: `PBL_IF_COLOR_ELSE()` fallback for segment colors

## Testing

### Unit testable (energy_logic.c)

- Segment count at various times throughout the day
- Color assignment per segment
- Midnight wrap-around math
- Boundary conditions (exactly at wake/sleep time)
- Time and date string formatting

### Emulator testing

- Visual appearance on emery (rect) and gabbro (round)
- Tap reveal show/hide cycle
- Pulse animation smoothness and focus pause/resume
- Config page flow on phone side
