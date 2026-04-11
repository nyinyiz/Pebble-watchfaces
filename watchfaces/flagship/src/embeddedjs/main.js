import Poco from "commodetto/Poco";
import { buildAnimationState } from "animation";

const render = new Poco(screen);

/* ─── Fonts (names/sizes from Pebble system font registry) ──────────────────
 *   "Leco-Regular",  42  →  FONT_KEY_LECO_42_NUMBERS    (LCD-style digits)
 *   "Gothic-Bold",   24  →  FONT_KEY_GOTHIC_24_BOLD      (date label)
 *   "Gothic-Bold",   18  →  FONT_KEY_GOTHIC_18_BOLD      (status labels)
 * ─────────────────────────────────────────────────────────────────────────── */
const timeFont   = new render.Font("Leco-Regular", 42);
const dateFont   = new render.Font("Gothic-Bold",  24);
const statusFont = new render.Font("Gothic-Bold",  18);

/* ─── Colour palette ─────────────────────────────────────────────────────── */
const C_BG        = render.makeColor(0,   0,   0);
const C_WHITE     = render.makeColor(255, 255, 255);
const C_DIM       = render.makeColor(100, 106, 128);
const C_BATT_RAIL = render.makeColor(22,  22,  32);
const C_DANGER    = render.makeColor(235, 87,  87);
const C_SNOW      = render.makeColor(210, 230, 255);  // cool white for snow caps

// Aurora bands — back-to-front: violet → teal → vivid green
const C_AURORA = [
  render.makeColor(98,  58,  205),   // deep violet  (back)
  render.makeColor(50,  175, 200),   // teal          (mid)
  render.makeColor(50,  225, 115),   // vivid green   (front)
];

// Time-of-day accent
const C_DAWN  = render.makeColor(248, 184, 79);   // warm amber  06:00–11:59
const C_DAY   = render.makeColor(95,  208, 255);  // sky blue    12:00–17:59
const C_NIGHT = render.makeColor(128, 168, 245);  // cool violet 18:00–05:59

/* ─── Screen geometry ────────────────────────────────────────────────────── */
const W        = render.width;
const H        = render.height;
const IS_ROUND = W === H;
/* ─── Star field ─────────────────────────────────────────────────────────── */
// Positions in the aurora / sky zone (Y 5–65 — above the mountain line).
// Deliberately kept above Y=65 so most stars are not hidden by mountains.
const STARS_RECT = [
  [  8, 12], [ 25,  7], [ 50, 18], [ 75,  5], [ 98, 14],
  [125,  8], [150, 20], [178, 10], [194, 17],
  [ 15, 30], [ 40, 36], [ 70, 26], [100, 33], [128, 40],
  [155, 28], [184, 35],
  [ 10, 52], [ 45, 58], [ 85, 48], [115, 55], [148, 62],
  [180, 50],
];

const STARS = IS_ROUND
  ? STARS_RECT.map(([x, y]) => [Math.round(x * W / 200), y + 5])
  : STARS_RECT;

/* ─── Aurora configuration ───────────────────────────────────────────────── */
const AURORA_TOP = IS_ROUND ? 8  : 5;
const AURORA_BOT = IS_ROUND ? 95 : 85;   // aurora is clipped here

// Three sine-wave bands drawn back-to-front.
const BANDS = [
  { baseY: IS_ROUND ? 78 : 68, amp: 7,  halfH: 5, freq: 0.035, speed: 0.110 },
  { baseY: IS_ROUND ? 58 : 48, amp: 10, halfH: 7, freq: 0.025, speed: 0.165 },
  { baseY: IS_ROUND ? 35 : 28, amp: 9,  halfH: 5, freq: 0.018, speed: 0.140 },
];

/* ─── Mountain silhouette ────────────────────────────────────────────────── */
// Mountains sit at the bottom of the aurora zone and create a dark landscape
// silhouette against the lights — each peak is a parabolic bump.
// [center_x, peak_height_in_px, spread_in_px]
const MTN_PEAKS = IS_ROUND
  ? [[50, 14, 24], [95, 12, 18], [132, 20, 30], [180, 13, 20], [230, 10, 16]]
  : [[20, 12, 20], [58, 10, 16], [102, 17, 26], [148, 11, 18], [185,  8, 14]];

const MTN_BASE = AURORA_BOT;   // mountains sit on top of aurora bottom edge

// Pre-compute mountain profile once at startup — lookup table avoids per-frame
// arithmetic cost that would otherwise be O(W × peaks) every draw call.
// Only built for rectangular screens; round display uses a flat horizon.
function buildMtnProfile() {
  if (IS_ROUND) return null;
  const profile = [];
  for (let x = 0; x < W; x++) {
    let h = 3;   // minimum terrain height
    for (let i = 0; i < MTN_PEAKS.length; i++) {
      const cx     = MTN_PEAKS[i][0];
      const ph     = MTN_PEAKS[i][1];
      const spread = MTN_PEAKS[i][2];
      const d = x - cx;
      if (d > -spread && d < spread) {
        const v = Math.round(ph * (1 - (d * d) / (spread * spread)));
        if (v > h) h = v;
      }
    }
    profile[x] = h;
  }
  return profile;
}
const MTN_PROFILE = buildMtnProfile();

/* ─── Layout positions ───────────────────────────────────────────────────── */
const DIVIDER_Y = IS_ROUND ? 100 : 88;
const LAYOUT = buildLayout({ width: W, height: H, isRound: IS_ROUND });

/* ─── Pure helpers ───────────────────────────────────────────────────────── */
function formatTime(date, is24Hour) {
  const hours24 = date.getHours();
  const minutes = String(date.getMinutes()).padStart(2, "0");

  if (is24Hour) return `${hours24}:${minutes}`;
  return `${hours24 % 12 || 12}:${minutes}`;
}

function formatDate(date) {
  const weekdays = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  const months = ["JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"];
  return `${weekdays[date.getDay()]} ${date.getDate()} ${months[date.getMonth()]}`;
}

function formatBattery(percent) {
  return `${Math.round(percent)}%`;
}

function formatConnection(isConnected) {
  return isConnected ? "BT OK" : "BT OFF";
}

function accentFor(hours) {
  if (hours < 6 || hours >= 18) return C_NIGHT;
  if (hours < 12) return C_DAWN;
  return C_DAY;
}

function buildLayout(bounds) {
  const centerX = Math.round(bounds.width / 2);

  if (bounds.isRound) {
    return {
      label: { x: centerX, y: 30 },
      meridiem: { x: centerX, y: 54 },
      time: { x: centerX, y: 110 },
      date: { x: centerX, y: 154 },
      statusLeft: { x: 44, y: 192 },
      statusCenter: { x: centerX, y: 192 },
      statusRight: { x: bounds.width - 44, y: 192 }
    };
  }

  return {
    label: { x: centerX, y: 42 },
    meridiem: { x: centerX, y: 70 },
    time: { x: centerX, y: 96 },
    date: { x: centerX, y: 128 },
    statusLeft: { x: 34, y: 176 },
    statusCenter: { x: centerX, y: 176 },
    statusRight: { x: bounds.width - 34, y: 176 }
  };
}

function buildFaceModel(state) {
  const hours = state.date.getHours();
  const batteryBase = formatBattery(state.batteryPercent);
  let batteryText = batteryBase;

  if (state.isCharging) {
    batteryText = `${batteryBase} CHG`;
  } else if (state.batteryPercent <= 20) {
    batteryText = `${batteryBase} LOW`;
  }

  return {
    palette: {
      accentName: hours < 6 || hours >= 18 ? "night" : hours < 12 ? "morning" : "day"
    },
    time: {
      text: formatTime(state.date, state.is24Hour),
      meridiem: state.is24Hour ? "" : hours >= 12 ? "PM" : "AM"
    },
    date: {
      text: formatDate(state.date)
    },
    statusLeft: {
      text: batteryText
    },
    statusCenter: {
      text: `SEC ${String(state.date.getSeconds()).padStart(2, "0")}`
    },
    statusRight: {
      text: formatConnection(state.isConnected)
    }
  };
}

/* ─── Sub-renderers ──────────────────────────────────────────────────────── */
function drawStars(animation) {
  for (let i = 0; i < STARS.length; i++) {
    const [x, y] = STARS[i];
    const dimmed = (i + animation.twinkleOffset) % 5 === 0;
    const col    = dimmed ? C_AURORA[1] : C_WHITE;
    const sz     = (!dimmed && i % 4 === 1) ? 2 : 1;
    render.fillRectangle(col, x, y, sz, sz);
  }
}

function drawAurora(animation) {
  const step = 2;

  for (let b = 0; b < BANDS.length; b++) {
    const { baseY, amp, halfH, freq } = BANDS[b];
    const phase = animation.bandPhases[b];
    const animatedHalfH = halfH + (b === 1 ? Math.round(animation.pulse * 2) : 0);

    for (let x = 0; x < W; x += step) {
      const edge = x < 15 ? x : (W - 1 - x < 15 ? W - 1 - x : 15);
      const hh   = Math.round(animatedHalfH * edge / 15);
      if (hh <= 0) continue;

      const yc = Math.round(baseY + Math.sin(x * freq + phase) * amp);
      const y0 = Math.max(yc - hh, AURORA_TOP);
      const y1 = Math.min(yc + hh, AURORA_BOT);
      if (y1 > y0) {
        render.fillRectangle(C_AURORA[b], x, y0, Math.min(step, W - x), y1 - y0);
      }
    }
  }
}

function drawSecondSweep(animation, acc) {
  const pulseLeft = Math.max(
    animation.sweepInset,
    animation.sweepX - animation.sweepHalfWidth
  );
  const pulseRight = Math.min(
    W - animation.sweepInset,
    animation.sweepX + animation.sweepHalfWidth
  );
  const pulseWidth = Math.max(1, pulseRight - pulseLeft);

  render.fillRectangle(acc, pulseLeft, DIVIDER_Y - 1, pulseWidth, 3);

  if (pulseLeft > animation.sweepInset) {
    render.drawLine(animation.sweepInset, DIVIDER_Y, pulseLeft, DIVIDER_Y, C_DIM, 1);
  }
}

function drawMountains() {
  if (!MTN_PROFILE) {
    // Round display: flat dark horizon strip — clean and avoids startup cost
    render.fillRectangle(C_BG, 0, MTN_BASE - 4, W, DIVIDER_Y - MTN_BASE + 4);
    return;
  }

  // Rect display: full mountain silhouette
  for (let x = 0; x < W; x += 2) {
    const h = Math.max(MTN_PROFILE[x], MTN_PROFILE[x + 1] || 0);
    render.fillRectangle(C_BG, x, MTN_BASE - h, Math.min(2, W - x), DIVIDER_Y - MTN_BASE + h);
  }

  // Snow caps — tiny highlight on every local peak above the snow line.
  let prevH = MTN_PROFILE[0];
  for (let x = 1; x < W - 1; x++) {
    const h = MTN_PROFILE[x];
    if (h > prevH && h >= MTN_PROFILE[x + 1] && h >= 9) {
      render.fillRectangle(C_SNOW, x - 1, MTN_BASE - h - 1, 3, 2);
    }
    prevH = h;
  }
}

function drawBatteryBar(pct, charging) {
  const barW = Math.max(2, Math.round(W * pct / 100));
  const col  = charging    ? C_AURORA[1]
             : pct > 30    ? C_AURORA[2]
             : pct > 15    ? C_DAWN
             :               C_DANGER;
  render.fillRectangle(C_BATT_RAIL, 0, 0, W, 3);
  render.fillRectangle(col,         0, 0, barW, 3);
}

/* ─── Main render ────────────────────────────────────────────────────────── */
function draw(event) {
  const now      = event?.date ?? new Date();
  const animation = buildAnimationState({ date: now, width: W, isRound: IS_ROUND });
  const pct      = typeof watch?.battery  === "number"  ? watch.battery  : 100;
  const charging = typeof watch?.charging === "boolean" ? watch.charging : false;
  const is24h    = watch?.timeStyle !== "12h";
  const connected = watch?.connected !== false;
  const model = buildFaceModel({
    date: now,
    is24Hour: is24h,
    batteryPercent: pct,
    isCharging: charging,
    isConnected: connected
  });
  const acc = accentFor(now.getHours());

  /* — Strings — */
  const timeStr = model.time.text;
  const meridStr = model.time.meridiem;
  const dateStr = model.date.text;
  const batStr = model.statusLeft.text;
  const secStr = model.statusCenter.text;
  const connStr = model.statusRight.text;

  /* — Render — */
  render.begin();

  // 1. Black background
  render.fillRectangle(C_BG, 0, 0, W, H);

  // 2. Stars (behind aurora)
  drawStars(animation);

  // 3. Aurora bands — violet, then teal, then green on top
  drawAurora(animation);

  // 4. Mountain silhouette — black overlay on the lower aurora zone, with snow caps
  drawMountains();

  // 5. Thin accent divider between sky and dial
  render.drawLine(animation.sweepInset, DIVIDER_Y, W - animation.sweepInset, DIVIDER_Y, acc, 1);
  drawSecondSweep(animation, acc);

  // 6. Time — LCD digits, white, centred
  const timeW = render.getTextWidth(timeStr, timeFont);
  const timeX = Math.round(LAYOUT.time.x - (timeW / 2));
  render.drawText(timeStr, timeFont, C_WHITE, timeX, LAYOUT.time.y);

  // 7. Meridiem — accent, right of time, lower baseline
  if (meridStr) {
    const mW = render.getTextWidth(meridStr, dateFont);
    const mX = timeX + timeW + 5;
    const mY = LAYOUT.time.y + 22;
    if (mX + mW < W - 2) {
      render.drawText(meridStr, dateFont, acc, mX, mY);
    }
  }

  // 8. Date
  const dateW = render.getTextWidth(dateStr, dateFont);
  render.drawText(dateStr, dateFont, acc,
    Math.round(LAYOUT.date.x - (dateW / 2)), LAYOUT.date.y);

  // 9. Status row
  const batW    = render.getTextWidth(batStr,  statusFont);
  const secW    = render.getTextWidth(secStr,  statusFont);
  const connW   = render.getTextWidth(connStr, statusFont);
  const batCol  = pct <= 20  ? C_DANGER : C_DIM;
  const secCol  = acc;
  const connCol = connected  ? C_DIM    : C_DANGER;
  render.drawText(batStr,  statusFont, batCol,
    Math.round(LAYOUT.statusLeft.x - (batW / 2)), LAYOUT.statusLeft.y);
  render.drawText(secStr,  statusFont, secCol,
    Math.round(LAYOUT.statusCenter.x - (secW / 2)), LAYOUT.statusCenter.y);
  render.drawText(connStr, statusFont, connCol,
    Math.round(LAYOUT.statusRight.x - (connW / 2)), LAYOUT.statusRight.y);

  // 10. Battery bar — always on top
  drawBatteryBar(pct, charging);

  render.end();
}

/* ─── Events ─────────────────────────────────────────────────────────────── */
watch.addEventListener("secondchange",              draw);
watch.addEventListener("wake",                      draw);
watch.addEventListener("batterychange",             draw);
watch.addEventListener("chargingchange",            draw);
watch.addEventListener("bluetoothconnectionchange", draw);

draw({ date: new Date() });
