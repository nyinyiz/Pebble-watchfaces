# Aurora Multi-Platform Support Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate Aurora from Moddable EmbeddedJS (emery + gabbro only) to Rocky.js, supporting all 5 color Pebble platforms: basalt, chalk, diorite, emery, gabbro.

**Architecture:** Replace `src/embeddedjs/` with `src/rocky/main.js` — a single self-contained Rocky.js rendering engine using the HTML5 Canvas 2D API. The existing `src/common/` logic modules (layout, format, model, animation) are kept; theme.mjs is promoted from `embeddedjs/` to `common/`. Rocky.js is loaded by the Pebble firmware natively; no C wrapper is needed.

**Tech Stack:** Rocky.js (Pebble on-device JS runtime), Pebble SDK 4.9.148, Node.js test runner for unit tests.

---

## File Map

| Action | Path | Purpose |
|---|---|---|
| Modify | `src/common/layout.mjs` | Add small-rect (144×168) and small-round (180×180) profiles |
| Move | `src/embeddedjs/theme.mjs` → `src/common/theme.mjs` | Make theme pure/testable from common |
| Modify | `tests/theme.test.mjs` | Update import path to `src/common/theme.mjs` |
| Modify | `tests/layout.test.mjs` | Add tests for the two new layout profiles |
| Create | `src/rocky/main.js` | Rocky.js rendering engine (replaces embeddedjs/main.js) |
| Modify | `package.json` | `projectType: rocky`, 5 target platforms |
| Modify | `wscript` | Remove Moddable build chain; standard Pebble WAF |
| Delete | `src/embeddedjs/` | Entire directory |
| Delete | `src/c/` | Entire directory (no C wrapper needed for Rocky.js) |

---

## Task 1: Extend layout.mjs with small-rect and small-round profiles

**Files:**
- Modify: `watchfaces/aurora/src/common/layout.mjs`
- Modify: `watchfaces/aurora/tests/layout.test.mjs`

- [ ] **Step 1: Add two failing tests for the new profiles**

Open `watchfaces/aurora/tests/layout.test.mjs` and append:

```js
test('buildLayout fits small-rect screens (144×168: basalt, diorite)', () => {
  const layout = buildLayout({ width: 144, height: 168, isRound: false });

  assert.deepEqual(layout.time,         { x: 72, y: 76  });
  assert.deepEqual(layout.date,         { x: 72, y: 106 });
  assert.deepEqual(layout.statusLeft,   { x: 24, y: 144 });
  assert.deepEqual(layout.statusCenter, { x: 72, y: 144 });
  assert.deepEqual(layout.statusRight,  { x: 120, y: 144 });
  assert.deepEqual(layout.label,        { x: 72, y: 30  });
});

test('buildLayout fits small-round screens (180×180: chalk)', () => {
  const layout = buildLayout({ width: 180, height: 180, isRound: true });

  assert.deepEqual(layout.time,         { x: 90, y: 80  });
  assert.deepEqual(layout.date,         { x: 90, y: 112 });
  assert.deepEqual(layout.statusLeft,   { x: 30, y: 140 });
  assert.deepEqual(layout.statusCenter, { x: 90, y: 140 });
  assert.deepEqual(layout.statusRight,  { x: 150, y: 140 });
  assert.deepEqual(layout.label,        { x: 90, y: 22  });
});
```

- [ ] **Step 2: Run the tests to confirm they fail**

```bash
cd watchfaces/aurora && npm test 2>&1 | grep -E 'fail|FAIL|pass|Error'
```

Expected: 2 failures for the new layout tests.

- [ ] **Step 3: Replace layout.mjs with 4-profile implementation**

Overwrite `watchfaces/aurora/src/common/layout.mjs`:

```js
export function buildLayout({ width, height, isRound }) {
  const cx = Math.round(width / 2);

  if (isRound && width >= 250) {
    // large-round: gabbro 260×260
    return {
      label:        { x: cx,           y: 30  },
      time:         { x: cx,           y: 110 },
      date:         { x: cx,           y: 154 },
      statusLeft:   { x: 44,           y: 192 },
      statusCenter: { x: cx,           y: 192 },
      statusRight:  { x: width - 44,   y: 192 },
    };
  }

  if (isRound) {
    // small-round: chalk 180×180
    return {
      label:        { x: cx,           y: 22  },
      time:         { x: cx,           y: 80  },
      date:         { x: cx,           y: 112 },
      statusLeft:   { x: 30,           y: 140 },
      statusCenter: { x: cx,           y: 140 },
      statusRight:  { x: width - 30,   y: 140 },
    };
  }

  if (width >= 190) {
    // large-rect: emery 200×228
    return {
      label:        { x: cx,           y: 42  },
      time:         { x: cx,           y: 96  },
      date:         { x: cx,           y: 128 },
      statusLeft:   { x: 34,           y: 176 },
      statusCenter: { x: cx,           y: 176 },
      statusRight:  { x: width - 34,   y: 176 },
    };
  }

  // small-rect: basalt/diorite 144×168
  return {
    label:        { x: cx,           y: 30  },
    time:         { x: cx,           y: 76  },
    date:         { x: cx,           y: 106 },
    statusLeft:   { x: 24,           y: 144 },
    statusCenter: { x: cx,           y: 144 },
    statusRight:  { x: width - 24,   y: 144 },
  };
}
```

- [ ] **Step 4: Run tests — all must pass**

```bash
cd watchfaces/aurora && npm test
```

Expected: all tests pass (existing 2 layout tests + 2 new ones).

- [ ] **Step 5: Commit**

```bash
git add watchfaces/aurora/src/common/layout.mjs watchfaces/aurora/tests/layout.test.mjs
git commit -m "feat(aurora): extend layout.mjs with small-rect and small-round profiles"
```

---

## Task 2: Promote theme.mjs to src/common and update tests

**Files:**
- Copy: `watchfaces/aurora/src/embeddedjs/theme.mjs` → `watchfaces/aurora/src/common/theme.mjs`
- Modify: `watchfaces/aurora/tests/theme.test.mjs`

- [ ] **Step 1: Copy theme.mjs to src/common/**

```bash
cp watchfaces/aurora/src/embeddedjs/theme.mjs watchfaces/aurora/src/common/theme.mjs
```

- [ ] **Step 2: Update the import in theme.test.mjs**

In `watchfaces/aurora/tests/theme.test.mjs`, change line 4 from:

```js
import { getThemeForHour, getThemeNameForHour } from '../src/embeddedjs/theme.mjs';
```

to:

```js
import { getThemeForHour, getThemeNameForHour } from '../src/common/theme.mjs';
```

- [ ] **Step 3: Run tests to confirm they still pass**

```bash
cd watchfaces/aurora && npm test
```

Expected: all tests pass.

- [ ] **Step 4: Commit**

```bash
git add watchfaces/aurora/src/common/theme.mjs watchfaces/aurora/tests/theme.test.mjs
git commit -m "refactor(aurora): promote theme.mjs to src/common"
```

---

## Task 3: Write src/rocky/main.js

**Files:**
- Create: `watchfaces/aurora/src/rocky/main.js`

Rocky.js key facts for this file:
- Entry point loaded by Pebble firmware automatically via `require('rocky')`
- Draw context is an HTML5 Canvas 2D-like API
- `ctx.canvas.clientWidth` / `ctx.canvas.clientHeight` give screen dimensions
- `event.date` in draw events is the current `Date`
- Battery: `rocky.watchInfo.battery()` → `{ chargePercent, isCharging }`
- Bluetooth: `rocky.watchInfo.bluetooth.connected` (boolean)
- Clock format: `rocky.preferences.clockFormat()` → `'12h'` or `'24h'`
- Fonts: CSS-like strings — `'42px Leco'`, `'bold 18px Gothic'`, `'bold 14px Gothic'`
- Text baseline defaults to top in Pebble Rocky.js (set explicitly to be safe)
- Module system: CommonJS `require()` — no ES `import`

- [ ] **Step 1: Create src/rocky/main.js**

Create `watchfaces/aurora/src/rocky/main.js` with the following content:

```js
'use strict';

var rocky = require('rocky');

// ── Color helpers ─────────────────────────────────────────────────────────────

function toHex(rgb) {
  return '#' + rgb.map(function(v) {
    return ('0' + v.toString(16)).slice(-2);
  }).join('');
}

// ── Theme ─────────────────────────────────────────────────────────────────────

var THEMES = {
  morning: {
    accent: [248, 184,  79],
    aurora: [[118, 162, 255], [94, 226, 199], [255, 206, 120]],
  },
  day: {
    accent: [ 95, 208, 255],
    aurora: [[ 78, 128, 255], [71, 213, 255], [112, 255, 184]],
  },
  evening: {
    accent: [255, 140,  92],
    aurora: [[126,  78, 221], [214,  94, 192], [255, 158,  96]],
  },
  night: {
    accent: [128, 168, 245],
    aurora: [[ 98,  58, 205], [ 50, 175, 200], [ 50, 225, 115]],
  },
};

var activeThemeName = 'night';
var accentColor  = toHex(THEMES.night.accent);
var auroraColors = THEMES.night.aurora.map(toHex);

function applyThemeForHour(hours) {
  var name = (hours < 6) ? 'night'
           : (hours < 12) ? 'morning'
           : (hours < 17) ? 'day'
           : (hours < 20) ? 'evening'
           : 'night';
  if (name === activeThemeName) { return; }
  activeThemeName = name;
  var t = THEMES[name];
  accentColor  = toHex(t.accent);
  auroraColors = t.aurora.map(toHex);
}

// ── Animation state ───────────────────────────────────────────────────────────

var TWO_PI          = Math.PI * 2;
var BAND_MULTIPLIERS = [0.85, 1.25, 1.65];
var BAND_OFFSETS     = [0.2,  1.1,  2.4];

function buildAnimationState(date, width, isRound) {
  var seconds       = date.getSeconds();
  var secondProgress = seconds / 60;
  var sweepInset     = isRound ? 40 : 12;
  var sweepHalfWidth = isRound ?  9 :  7;
  var trackWidth     = Math.max(0, width - sweepInset * 2);
  var sweepX         = sweepInset + Math.round(trackWidth * secondProgress);
  var bandPhases     = BAND_MULTIPLIERS.map(function(m, i) {
    return ((secondProgress * TWO_PI * m) + BAND_OFFSETS[i]) % TWO_PI;
  });
  return {
    secondProgress:  secondProgress,
    sweepInset:      sweepInset,
    sweepHalfWidth:  sweepHalfWidth,
    sweepX:          sweepX,
    bandPhases:      bandPhases,
    twinkleOffset:   seconds % 5,
    pulse:           Math.sin(secondProgress * TWO_PI),
  };
}

// ── Mountain profile ──────────────────────────────────────────────────────────

function buildMountainProfile(width, peaks) {
  var profile = [];
  for (var x = 0; x < width; x++) {
    var height = 3;
    for (var i = 0; i < peaks.length; i++) {
      var px = peaks[i][0], ph = peaks[i][1], ps = peaks[i][2];
      var d  = x - px;
      if (d > -ps && d < ps) {
        var c = Math.round(ph * (1 - (d * d) / (ps * ps)));
        if (c > height) { height = c; }
      }
    }
    profile[x] = height;
  }
  return profile;
}

// ── Layout ────────────────────────────────────────────────────────────────────

function buildLayout(width, isRound) {
  var cx = Math.round(width / 2);

  if (isRound && width >= 250) {
    return {
      label:        { x: cx,           y:  30 },
      time:         { x: cx,           y: 110 },
      date:         { x: cx,           y: 154 },
      statusLeft:   { x: 44,           y: 192 },
      statusCenter: { x: cx,           y: 192 },
      statusRight:  { x: width - 44,   y: 192 },
    };
  }

  if (isRound) {
    return {
      label:        { x: cx,           y:  22 },
      time:         { x: cx,           y:  80 },
      date:         { x: cx,           y: 112 },
      statusLeft:   { x: 30,           y: 140 },
      statusCenter: { x: cx,           y: 140 },
      statusRight:  { x: width - 30,   y: 140 },
    };
  }

  if (width >= 190) {
    return {
      label:        { x: cx,           y:  42 },
      time:         { x: cx,           y:  96 },
      date:         { x: cx,           y: 128 },
      statusLeft:   { x: 34,           y: 176 },
      statusCenter: { x: cx,           y: 176 },
      statusRight:  { x: width - 34,   y: 176 },
    };
  }

  return {
    label:        { x: cx,           y:  30 },
    time:         { x: cx,           y:  76 },
    date:         { x: cx,           y: 106 },
    statusLeft:   { x: 24,           y: 144 },
    statusCenter: { x: cx,           y: 144 },
    statusRight:  { x: width - 24,   y: 144 },
  };
}

// ── Initialized state (computed once on first draw) ───────────────────────────

var W, H, IS_ROUND, SHOW_MOUNTAINS;
var STARS, AURORA_TOP, AURORA_BOT, BANDS;
var MTN_PEAKS, MTN_BASE, MTN_PROFILE, DIVIDER_Y, LAYOUT;

function init(ctx) {
  W         = ctx.canvas.clientWidth;
  H         = ctx.canvas.clientHeight;
  IS_ROUND  = (W === H);
  var small = (W <= 180);

  // Mountains on every platform except gabbro (260×260)
  SHOW_MOUNTAINS = (W !== 260);

  // Star positions — base grid at 200px width, scaled to actual W
  STARS = [
    [  8, 12], [ 25,  7], [ 50, 18], [ 75,  5], [ 98, 14],
    [125,  8], [150, 20], [178, 10], [194, 17],
    [ 15, 30], [ 40, 36], [ 70, 26], [100, 33], [128, 40],
    [155, 28], [184, 35],
    [ 10, 52], [ 45, 58], [ 85, 48], [115, 55], [148, 62],
    [180, 50],
  ].map(function(s) {
    return [
      Math.round(s[0] * W / 200),
      Math.round(s[1] * (small ? 0.72 : 1)),
    ];
  });

  AURORA_TOP = IS_ROUND ? 8 : 5;
  if (small && IS_ROUND)  { AURORA_BOT = 68; }
  else if (small)         { AURORA_BOT = 62; }
  else if (IS_ROUND)      { AURORA_BOT = 95; }
  else                    { AURORA_BOT = 85; }

  if (IS_ROUND && !small) {
    BANDS = [
      { baseY: 78, amp: 7,  halfH: 5, freq: 0.035 },
      { baseY: 58, amp: 10, halfH: 7, freq: 0.025 },
      { baseY: 35, amp: 9,  halfH: 5, freq: 0.018 },
    ];
  } else if (small && IS_ROUND) {
    BANDS = [
      { baseY: 58, amp: 6, halfH: 4, freq: 0.038 },
      { baseY: 42, amp: 9, halfH: 6, freq: 0.026 },
      { baseY: 24, amp: 8, halfH: 4, freq: 0.019 },
    ];
  } else if (small) {
    BANDS = [
      { baseY: 54, amp: 5, halfH: 4, freq: 0.04  },
      { baseY: 38, amp: 8, halfH: 6, freq: 0.028 },
      { baseY: 22, amp: 7, halfH: 4, freq: 0.02  },
    ];
  } else {
    BANDS = [
      { baseY: 68, amp: 7,  halfH: 5, freq: 0.035 },
      { baseY: 48, amp: 10, halfH: 7, freq: 0.025 },
      { baseY: 28, amp: 9,  halfH: 5, freq: 0.018 },
    ];
  }

  if (IS_ROUND && !small) {
    MTN_PEAKS = [[50,14,24],[95,12,18],[132,20,30],[180,13,20],[230,10,16]];
  } else if (small && IS_ROUND) {
    MTN_PEAKS = [[35,11,17],[66,9,12],[91,14,21],[124,9,14],[159,7,11]];
  } else if (small) {
    MTN_PEAKS = [[14,10,14],[42,8,12],[73,14,19],[107,9,13],[133,7,10]];
  } else {
    MTN_PEAKS = [[20,12,20],[58,10,16],[102,17,26],[148,11,18],[185,8,14]];
  }

  MTN_BASE    = AURORA_BOT;
  MTN_PROFILE = SHOW_MOUNTAINS ? buildMountainProfile(W, MTN_PEAKS) : null;

  if (small && IS_ROUND) { DIVIDER_Y = 72; }
  else if (small)        { DIVIDER_Y = 70; }
  else if (IS_ROUND)     { DIVIDER_Y = 100; }
  else                   { DIVIDER_Y = 88; }

  LAYOUT = buildLayout(W, IS_ROUND);
}

// ── Runtime state ─────────────────────────────────────────────────────────────

var batteryPercent = 100;
var isCharging     = false;

// ── Formatters ────────────────────────────────────────────────────────────────

function formatTime(date, is24Hour) {
  var h = date.getHours();
  var m = ('0' + date.getMinutes()).slice(-2);
  if (is24Hour) { return h + ':' + m; }
  return (h % 12 || 12) + ':' + m;
}

function formatDate(date) {
  var days   = ['SUN','MON','TUE','WED','THU','FRI','SAT'];
  var months = ['JAN','FEB','MAR','APR','MAY','JUN',
                'JUL','AUG','SEP','OCT','NOV','DEC'];
  return days[date.getDay()] + ' ' + date.getDate() + ' ' + months[date.getMonth()];
}

// ── Draw helpers ──────────────────────────────────────────────────────────────

var C_BG        = '#000000';
var C_WHITE     = '#ffffff';
var C_DIM       = '#646a80';
var C_DANGER    = '#eb5757';
var C_SNOW      = '#d2e6ff';
var C_BATT_RAIL = '#161620';

function fillRect(ctx, color, x, y, w, h) {
  ctx.fillStyle = color;
  ctx.fillRect(x, y, w, h);
}

function drawLine(ctx, color, x1, y1, x2, y2) {
  ctx.strokeStyle = color;
  ctx.lineWidth   = 1;
  ctx.beginPath();
  ctx.moveTo(x1, y1);
  ctx.lineTo(x2, y2);
  ctx.stroke();
}

// ── Sub-renderers ─────────────────────────────────────────────────────────────

function drawStars(ctx, anim) {
  for (var i = 0; i < STARS.length; i++) {
    var x      = STARS[i][0], y = STARS[i][1];
    var dimmed = (i + anim.twinkleOffset) % 5 === 0;
    var color  = dimmed ? auroraColors[1] : C_WHITE;
    var size   = (!dimmed && i % 4 === 1) ? 2 : 1;
    fillRect(ctx, color, x, y, size, size);
  }
}

function drawAurora(ctx, anim) {
  var step = 2;
  for (var b = 0; b < BANDS.length; b++) {
    var band     = BANDS[b];
    var phase    = anim.bandPhases[b];
    var animHalfH = band.halfH + (b === 1 ? Math.round(anim.pulse * 2) : 0);
    for (var x = 0; x < W; x += step) {
      var edge = x < 15 ? x : (W - 1 - x < 15 ? W - 1 - x : 15);
      var colH = Math.round(animHalfH * edge / 15);
      if (colH <= 0) { continue; }
      var cy   = Math.round(band.baseY + Math.sin(x * band.freq + phase) * band.amp);
      var topY = Math.max(cy - colH, AURORA_TOP);
      var botY = Math.min(cy + colH, AURORA_BOT);
      if (botY > topY) {
        fillRect(ctx, auroraColors[b], x, topY, Math.min(step, W - x), botY - topY);
      }
    }
  }
}

function drawMountains(ctx) {
  if (!MTN_PROFILE) { return; }
  for (var x = 0; x < W; x += 2) {
    var h = Math.max(MTN_PROFILE[x], MTN_PROFILE[x + 1] || 0);
    fillRect(ctx, C_BG, x, MTN_BASE - h, Math.min(2, W - x), DIVIDER_Y - MTN_BASE + h);
  }
  var prev = MTN_PROFILE[0];
  for (var x2 = 1; x2 < W - 1; x2++) {
    var h2 = MTN_PROFILE[x2];
    if (h2 > prev && h2 >= MTN_PROFILE[x2 + 1] && h2 >= 9) {
      fillRect(ctx, C_SNOW, x2 - 1, MTN_BASE - h2 - 1, 3, 2);
    }
    prev = h2;
  }
}

function drawSecondSweep(ctx, anim) {
  var pulseLeft  = Math.max(anim.sweepInset, anim.sweepX - anim.sweepHalfWidth);
  var pulseRight = Math.min(W - anim.sweepInset, anim.sweepX + anim.sweepHalfWidth);
  var pulseWidth = Math.max(1, pulseRight - pulseLeft);
  fillRect(ctx, accentColor, pulseLeft, DIVIDER_Y - 1, pulseWidth, 3);
  if (pulseLeft > anim.sweepInset) {
    drawLine(ctx, C_DIM, anim.sweepInset, DIVIDER_Y, pulseLeft, DIVIDER_Y);
  }
}

function drawBatteryBar(ctx) {
  var barWidth = Math.max(2, Math.round(W * batteryPercent / 100));
  var color = isCharging        ? auroraColors[1]
            : batteryPercent > 30 ? auroraColors[2]
            : batteryPercent > 15 ? accentColor
            : C_DANGER;
  fillRect(ctx, C_BATT_RAIL, 0, 0, W,        3);
  fillRect(ctx, color,       0, 0, barWidth,  3);
}

// ── Main draw ─────────────────────────────────────────────────────────────────

rocky.on('draw', function(event) {
  var ctx = event.context;
  if (!W) { init(ctx); }

  var now  = event.date || new Date();
  var small = (W <= 180);

  applyThemeForHour(now.getHours());

  var anim        = buildAnimationState(now, W, IS_ROUND);
  var is24Hour    = rocky.preferences.clockFormat() === '24h';
  var isConnected = rocky.watchInfo.bluetooth.connected;
  var bat         = rocky.watchInfo.battery();
  batteryPercent  = bat.chargePercent;
  isCharging      = bat.isCharging;

  var timeFont   = small ? '28px Leco'          : '42px Leco';
  var dateFont   = small ? 'bold 14px Gothic'   : 'bold 18px Gothic';
  var statusFont = small ? 'bold 14px Gothic'   : 'bold 18px Gothic';

  var timeText    = formatTime(now, is24Hour);
  var meridiemText = is24Hour ? '' : (now.getHours() >= 12 ? 'PM' : 'AM');
  var dateText    = formatDate(now);
  var pct         = batteryPercent;
  var batteryText = pct + '%' + (isCharging ? ' CHG' : pct <= 20 ? ' LOW' : '');
  var secondsText = 'SEC ' + ('0' + now.getSeconds()).slice(-2);
  var connText    = isConnected ? 'BT OK' : 'BT OFF';
  var battColor   = pct <= 20 ? C_DANGER : C_DIM;
  var connColor   = isConnected ? C_DIM : C_DANGER;

  // 1. Background
  ctx.fillStyle = C_BG;
  ctx.fillRect(0, 0, W, H);

  // 2. Stars
  drawStars(ctx, anim);

  // 3. Aurora bands
  drawAurora(ctx, anim);

  // 4. Mountains (all except gabbro)
  drawMountains(ctx);

  // 5. Divider line + second sweep
  drawLine(ctx, accentColor, anim.sweepInset, DIVIDER_Y, W - anim.sweepInset, DIVIDER_Y);
  drawSecondSweep(ctx, anim);

  // 6. Time (left-aligned at computed x so meridiem can be appended)
  ctx.textBaseline = 'top';
  ctx.textAlign    = 'left';
  ctx.font         = timeFont;
  var tw    = ctx.measureText(timeText).width;
  var timeX = LAYOUT.time.x - Math.round(tw / 2);
  ctx.fillStyle = C_WHITE;
  ctx.fillText(timeText, timeX, LAYOUT.time.y);

  // 7. Meridiem (to the right of time text)
  if (meridiemText) {
    ctx.font = dateFont;
    var mx = timeX + tw + 5;
    var my = LAYOUT.time.y + (small ? 14 : 22);
    if (mx + ctx.measureText(meridiemText).width < W - 2) {
      ctx.fillStyle = accentColor;
      ctx.fillText(meridiemText, mx, my);
    }
  }

  // 8. Date
  ctx.textAlign = 'center';
  ctx.font      = dateFont;
  ctx.fillStyle = accentColor;
  ctx.fillText(dateText, LAYOUT.date.x, LAYOUT.date.y);

  // 9. Status row
  ctx.font      = statusFont;
  ctx.textAlign = 'center';
  ctx.fillStyle = battColor;
  ctx.fillText(batteryText, LAYOUT.statusLeft.x,   LAYOUT.statusLeft.y);
  ctx.fillStyle = accentColor;
  ctx.fillText(secondsText, LAYOUT.statusCenter.x, LAYOUT.statusCenter.y);
  ctx.fillStyle = connColor;
  ctx.fillText(connText,    LAYOUT.statusRight.x,  LAYOUT.statusRight.y);

  // 10. Battery bar (top edge)
  drawBatteryBar(ctx);
});

// ── Subscriptions ─────────────────────────────────────────────────────────────

rocky.on('secondchange', function() { rocky.requestDraw(); });
rocky.on('minutechange', function() { rocky.requestDraw(); });
```

- [ ] **Step 2: Commit**

```bash
git add watchfaces/aurora/src/rocky/main.js
git commit -m "feat(aurora): add Rocky.js rendering engine for all 5 color platforms"
```

---

## Task 4: Update package.json and wscript

**Files:**
- Modify: `watchfaces/aurora/package.json`
- Modify: `watchfaces/aurora/wscript`

- [ ] **Step 1: Replace package.json**

Overwrite `watchfaces/aurora/package.json`:

```json
{
  "name": "aurora-watchface",
  "version": "0.1.0",
  "private": true,
  "description": "A premium utility-first Pebble watchface with an aurora borealis visual design.",
  "author": "nyinyiz",
  "license": "UNLICENSED",
  "scripts": {
    "test": "node --test tests/*.test.mjs"
  },
  "pebble": {
    "displayName": "Aurora",
    "projectType": "rocky",
    "sdkVersion": "3",
    "uuid": "00ded3ec-8b6e-45b5-8a50-a170e4000467",
    "enableMultiJS": true,
    "watchapp": {
      "watchface": true
    },
    "targetPlatforms": [
      "basalt",
      "chalk",
      "diorite",
      "emery",
      "gabbro"
    ],
    "resources": {
      "media": []
    }
  }
}
```

- [ ] **Step 2: Replace wscript**

Overwrite `watchfaces/aurora/wscript` with a standard Pebble WAF build (no Moddable XSC chain, no C wrapper):

```python
import os.path

top = '.'
out = 'build'


def options(ctx):
    ctx.load('pebble_sdk')


def configure(ctx):
    ctx.load('pebble_sdk')


def build(ctx):
    ctx.load('pebble_sdk')

    binaries = []

    cached_env = ctx.env
    for platform in ctx.env.TARGET_PLATFORMS:
        ctx.env = ctx.all_envs[platform]
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf = '{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_build(source=[], target=app_elf, bin_type='app')
        binaries.append({'platform': platform, 'app_elf': app_elf})
    ctx.env = cached_env

    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries,
                   js=ctx.path.ant_glob(['src/rocky/**/*.js',
                                         'src/pkjs/**/*.js']),
                   js_entry_file='src/pkjs/index.js')
```

- [ ] **Step 3: Commit**

```bash
git add watchfaces/aurora/package.json watchfaces/aurora/wscript
git commit -m "chore(aurora): switch to Rocky.js project type, add 5 target platforms"
```

---

## Task 5: Delete Moddable files and build

**Files:**
- Delete: `watchfaces/aurora/src/embeddedjs/`
- Delete: `watchfaces/aurora/src/c/`

- [ ] **Step 1: Delete src/embeddedjs/ and src/c/**

```bash
rm -rf watchfaces/aurora/src/embeddedjs
rm -rf watchfaces/aurora/src/c
```

- [ ] **Step 2: Run npm test to confirm tests still pass**

```bash
cd watchfaces/aurora && npm test
```

Expected: all tests pass (theme now loads from `src/common/theme.mjs`; no test imports from embeddedjs).

- [ ] **Step 3: Run pebble build**

```bash
cd watchfaces/aurora && ~/.local/bin/pebble build 2>&1
```

Expected: Build succeeds for all 5 platforms (basalt, chalk, diorite, emery, gabbro) with a `.pbw` produced at `build/aurora.pbw`.

If the build fails, the most likely errors and fixes are:

**Error: "no C source files"** — `pbl_build(source=[], ...)` may need at least one source. Fix: add a minimal stub:
```bash
mkdir -p watchfaces/aurora/src/c
echo '#include <pebble.h>' > watchfaces/aurora/src/c/stub.c
```
Then change `source=[]` to `source=ctx.path.ant_glob('src/c/**/*.c')` in wscript.

**Error: "Rocky.js entry file not found"** — The SDK may need the entry file specified differently. Check if changing `js_entry_file='src/pkjs/index.js'` to `js_entry_file='src/rocky/main.js'` fixes it, or if adding `"rocky": { "main": "src/rocky/main.js" }` to package.json is needed.

**Error: "unknown platform"** — `basalt`, `diorite`, `chalk` may require removing `"projectType": "rocky"` if the repebble SDK doesn't recognise it. Try without it.

**Runtime error: `rocky.watchInfo.battery is not a function`** — The SDK may expose battery as a property instead of a function. Change `rocky.watchInfo.battery()` to `rocky.watchInfo.battery` (no parentheses) in main.js.

- [ ] **Step 4: Commit**

```bash
git add -A watchfaces/aurora/
git commit -m "feat(aurora): migrate to Rocky.js, drop Moddable, support 5 color platforms"
```

---

## Task 6: Install, screenshot, and verify all 5 platforms

**Files:** `watchfaces/aurora/screenshots/`

- [ ] **Step 1: Install and screenshot basalt**

```bash
cd watchfaces/aurora
~/.local/bin/pebble install --emulator basalt
~/.local/bin/pebble screenshot --no-open --emulator basalt screenshots/basalt.png
```

- [ ] **Step 2: Install and screenshot chalk**

```bash
~/.local/bin/pebble install --emulator chalk
~/.local/bin/pebble screenshot --no-open --emulator chalk screenshots/chalk.png
```

- [ ] **Step 3: Install and screenshot diorite**

```bash
~/.local/bin/pebble install --emulator diorite
~/.local/bin/pebble screenshot --no-open --emulator diorite screenshots/diorite.png
```

- [ ] **Step 4: Install and screenshot emery**

```bash
~/.local/bin/pebble install --emulator emery
~/.local/bin/pebble screenshot --no-open --emulator emery screenshots/emery.png
```

- [ ] **Step 5: Install and screenshot gabbro**

```bash
~/.local/bin/pebble install --emulator gabbro
~/.local/bin/pebble screenshot --no-open --emulator gabbro screenshots/gabbro.png
```

- [ ] **Step 6: Visual check**

Review all 5 screenshots. Confirm:
- Aurora bands visible on all platforms
- Mountains visible on basalt, chalk, diorite, emery
- NO mountains on gabbro
- Time text readable on all platforms
- Status row fits without clipping on small (144×168) screens
- Round bezels clip correctly on chalk and gabbro

- [ ] **Step 7: Final commit with screenshots**

```bash
git add watchfaces/aurora/screenshots/
git commit -m "chore(aurora): add emulator screenshots for all 5 platforms"
```
