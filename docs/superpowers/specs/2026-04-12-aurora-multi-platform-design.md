# Aurora Multi-Platform Support Design

## Goal

Extend the Aurora watchface from 2 platforms (emery, gabbro) to all 5 Pebble color platforms (basalt, chalk, diorite, emery, gabbro) by migrating from Moddable EmbeddedJS to Rocky.js — a single rendering codebase that runs natively on every color Pebble.

---

## Platforms

| Platform | Device | Resolution | Shape |
|---|---|---|---|
| basalt | Pebble Time, Pebble Time Steel | 144×168 | Rect |
| chalk | Pebble Time Round | 180×180 | Round |
| diorite | Pebble 2, Pebble 2 SE | 144×168 | Rect |
| emery | Pebble Time 2 | 200×228 | Rect |
| gabbro | Pebble Time Round 2 | 260×260 | Round |

Aplite (OG Pebble, Steel) is excluded — B&W display, no JS runtime, and the aurora visual identity does not translate to 1-bit rendering.

---

## Architecture

### Project type change

`package.json` changes from `"projectType": "moddable"` to `"projectType": "rocky"` and `targetPlatforms` expands from `["emery", "gabbro"]` to `["basalt", "chalk", "diorite", "emery", "gabbro"]`.

### Files removed

- `src/embeddedjs/` — entire directory (Moddable-specific rendering)
- `src/c/mdbl.c` — Moddable C wrapper (Rocky.js has no C wrapper)

### Files added

- `src/rocky/main.js` — Rocky.js rendering engine (replaces `src/embeddedjs/main.js`)
- `src/rocky/theme.mjs` — theme system with CSS hex colors (replaces `src/embeddedjs/theme.mjs`)
- `src/rocky/animation.mjs` — copied from `src/embeddedjs/animation.mjs` (unchanged logic)
- `src/rocky/mountains.mjs` — copied from `src/embeddedjs/mountains.mjs` (unchanged logic)

### Files unchanged

- `src/common/format.mjs` — pure formatting functions, no runtime dependency
- `src/common/layout.mjs` — layout math (extended with new profiles, see Layout section)
- `src/common/model.mjs` — UI state aggregator, no runtime dependency
- `src/pkjs/index.js` — phone companion stub

### Files modified

- `src/common/layout.mjs` — two new layout profiles added (see Layout section)
- `wscript` — simplified to standard Pebble WAF; Rocky.js needs no XSC compiler chain
- `package.json` — `"projectType": "rocky"`, expanded `targetPlatforms`, add `"rocky": { "main": "src/rocky/main.js" }` entry

---

## Layout Profiles

Four screen profiles, determined by `width` and `height`:

| Profile | Trigger | Platforms |
|---|---|---|
| `small-rect` | width === 144 | basalt, diorite |
| `small-round` | width === 180 | chalk |
| `large-rect` | width === 200 | emery |
| `large-round` | width === 260 | gabbro |

`layout.mjs` exports `buildLayout({ width, height, isRound })` — emery and gabbro cases stay as-is; basalt/diorite and chalk cases are added.

---

## Rocky.js Rendering API Mapping

| Moddable | Rocky.js |
|---|---|
| `render.begin()` / `render.end()` | implicit inside `rocky.on('draw', ...)` |
| `render.makeColor(r, g, b)` | CSS hex string `'#RRGGBB'` |
| `render.fillRectangle(color, x, y, w, h)` | `ctx.fillStyle = color; ctx.fillRect(x, y, w, h)` |
| `render.drawText(text, font, color, x, y)` | `ctx.font = ...; ctx.fillStyle = color; ctx.fillText(text, x, y)` |
| `render.getTextWidth(text, font)` | `ctx.measureText(text).width` |
| `watch.connected?.app` | `rocky.watchInfo.bluetooth.connected` |
| `new Battery({onSample})` + `bat.sample()` | `rocky.watchInfo.battery()` → `{chargePercent, isCharging}` |
| `watch.hour12` | `rocky.preferences.clockFormat() === '12h'` |
| `watch.addEventListener('secondchange', h)` | `rocky.on('secondchange', h)` |
| `watch.addEventListener('minutechange', h)` | `rocky.on('minutechange', h)` |
| `render.width` / `render.height` | `ctx.canvas.clientWidth` / `ctx.canvas.clientHeight` |

---

## Draw Pipeline

Draw order is identical to the current Moddable implementation:

1. Black background fill
2. Star field (twinkling)
3. Aurora bands ×3 — sine-wave animated per second
4. Mountain silhouette — **all profiles including chalk (clipped by round bezel)**; gabbro only has no mountains
5. Accent divider line
6. Second-sweep pulse along divider
7. Time — large Leco digits, white, centred
8. Meridiem (AM/PM) — 12h mode only, accent colour
9. Date — "SUN 12 APR", accent colour
10. Status row — battery% | SEC XX | BT OK/OFF
11. Battery indicator bar (top edge)

---

## Theme System

Four time-of-day themes carry over unchanged in logic. Only the color representation changes: `render.makeColor(r, g, b)` → CSS hex strings.

| Time | Theme | Accent |
|---|---|---|
| 06:00–11:59 | morning | amber `#f8b84f` |
| 12:00–16:59 | day | sky blue `#5fd0ff` |
| 17:00–19:59 | evening | coral `#ff8c5c` |
| 20:00–05:59 | night | violet `#80a8f5` |

`applyThemeForHour(hours)` is called at the start of each draw — same guard (no-op if theme hasn't changed).

---

## Testing

### Existing tests (unchanged)

All 33 tests in `tests/` are pure JS with no runtime dependency and continue to pass as-is:
- `format.test.mjs` — 9 tests
- `layout.test.mjs` — existing emery/gabbro cases
- `model.test.mjs` — UI state tests
- `animation.test.mjs` — animation math tests
- `theme.test.mjs` — existing theme tests

### New / extended tests

- `tests/layout.test.mjs` — add cases for small-rect (144×168) and small-round (180×180): verify all layout values are positive, fit within their respective bounds, and `isRound` is correct per profile.
- `tests/theme.test.mjs` — verify all color values in all 4 themes are valid CSS hex strings (`/^#[0-9a-f]{6}$/i`).

### Visual validation

After build, take emulator screenshots on all 5 platforms:
```bash
pebble install --emulator basalt && pebble screenshot --emulator basalt screenshots/basalt.png
pebble install --emulator chalk  && pebble screenshot --emulator chalk  screenshots/chalk.png
pebble install --emulator diorite && pebble screenshot --emulator diorite screenshots/diorite.png
pebble install --emulator emery  && pebble screenshot --emulator emery  screenshots/emery.png
pebble install --emulator gabbro && pebble screenshot --emulator gabbro screenshots/gabbro.png
```

---

## Success Criteria

- `pebble build` succeeds with zero errors for all 5 platforms
- `npm test` passes all tests (33 existing + new layout/theme additions)
- Screenshots show correct visual on all 5 platforms: aurora animation visible, time readable, mountains present on basalt/chalk/diorite/emery, absent on gabbro
