import Poco from "commodetto/Poco";

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

/* ─── Animation frame counter ────────────────────────────────────────────── */
let frame = 0;

/* ─── Layout Y positions ─────────────────────────────────────────────────── */
const DIVIDER_Y = IS_ROUND ? 100 : 88;
const TIME_Y    = IS_ROUND ? 108 : 96;
const DATE_Y    = IS_ROUND ? 162 : 150;
const STATUS_Y  = IS_ROUND ? 204 : 192;

/* ─── Pure helpers ───────────────────────────────────────────────────────── */
function accentFor(hours) {
  if (hours < 6 || hours >= 18) return C_NIGHT;
  if (hours < 12) return C_DAWN;
  return C_DAY;
}

function pad2(n) { return n < 10 ? `0${n}` : `${n}`; }

/* ─── Sub-renderers ──────────────────────────────────────────────────────── */
function drawStars() {
  for (let i = 0; i < STARS.length; i++) {
    const [x, y] = STARS[i];
    const dimmed = (i + frame) % 5 === 0;
    const col    = dimmed ? C_AURORA[1] : C_WHITE;
    const sz     = (!dimmed && i % 4 === 1) ? 2 : 1;
    render.fillRectangle(col, x, y, sz, sz);
  }
}

function drawAurora() {
  for (let b = 0; b < BANDS.length; b++) {
    const { baseY, amp, halfH, freq, speed } = BANDS[b];
    const phase = frame * speed;

    for (let x = 0; x < W; x++) {
      const edge = x < 15 ? x : (W - 1 - x < 15 ? W - 1 - x : 15);
      const hh   = Math.round(halfH * edge / 15);
      if (hh <= 0) continue;

      const yc = Math.round(baseY + Math.sin(x * freq + phase) * amp);
      const y0 = Math.max(yc - hh, AURORA_TOP);
      const y1 = Math.min(yc + hh, AURORA_BOT);
      if (y1 > y0) {
        render.fillRectangle(C_AURORA[b], x, y0, 1, y1 - y0);
      }
    }
  }
}

function drawMountains() {
  if (!MTN_PROFILE) {
    // Round display: flat dark horizon strip — clean and avoids startup cost
    render.fillRectangle(C_BG, 0, MTN_BASE - 4, W, DIVIDER_Y - MTN_BASE + 4);
    return;
  }

  // Rect display: full mountain silhouette
  for (let x = 0; x < W; x++) {
    const h = MTN_PROFILE[x];
    render.fillRectangle(C_BG, x, MTN_BASE - h, 1, DIVIDER_Y - MTN_BASE + h);
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
function draw() {
  const now      = new Date();
  const hrs      = now.getHours();
  const acc      = accentFor(hrs);
  const pct      = typeof watch?.battery  === "number"  ? watch.battery  : 100;
  const charging = typeof watch?.charging === "boolean" ? watch.charging : false;
  const is24h    = watch?.timeStyle !== "12h";
  const connected = watch?.connected !== false;

  /* — Strings — */
  const h12     = hrs % 12 || 12;
  const minStr  = pad2(now.getMinutes());
  const timeStr = is24h ? `${hrs}:${minStr}` : `${h12}:${minStr}`;
  const meridStr = is24h ? "" : hrs >= 12 ? "PM" : "AM";

  const DAYS   = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  const MONTHS = ["JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"];
  const dateStr = `${DAYS[now.getDay()]}  ${now.getDate()}  ${MONTHS[now.getMonth()]}`;

  const batStr  = charging    ? `${Math.round(pct)}% +`
                : pct <= 20   ? `${Math.round(pct)}% !`
                :               `${Math.round(pct)}%`;
  const connStr = connected ? "ONLINE" : "OFFLINE";

  /* — Render — */
  render.begin();

  // 1. Black background
  render.fillRectangle(C_BG, 0, 0, W, H);

  // 2. Stars (behind aurora)
  drawStars();

  // 3. Aurora bands — violet, then teal, then green on top
  drawAurora();

  // 4. Mountain silhouette — black overlay on the lower aurora zone, with snow caps
  drawMountains();

  // 5. Thin accent divider between sky and dial
  const divInset = IS_ROUND ? 40 : 12;
  render.drawLine(divInset, DIVIDER_Y, W - divInset, DIVIDER_Y, acc, 1);

  // 6. Time — LCD digits, white, centred
  const timeW = render.getTextWidth(timeStr, timeFont);
  const timeX = Math.round((W - timeW) / 2);
  render.drawText(timeStr, timeFont, C_WHITE, timeX, TIME_Y);

  // 7. Meridiem — accent, right of time, lower baseline
  if (meridStr) {
    const mW = render.getTextWidth(meridStr, dateFont);
    const mX = timeX + timeW + 5;
    const mY = TIME_Y + 22;
    if (mX + mW < W - 2) {
      render.drawText(meridStr, dateFont, acc, mX, mY);
    }
  }

  // 8. Date
  const dateW = render.getTextWidth(dateStr, dateFont);
  render.drawText(dateStr, dateFont, acc, Math.round((W - dateW) / 2), DATE_Y);

  // 9. Status row
  const batW    = render.getTextWidth(batStr,  statusFont);
  const connW   = render.getTextWidth(connStr, statusFont);
  const batCol  = pct <= 20  ? C_DANGER : C_DIM;
  const connCol = connected  ? C_DIM    : C_DANGER;
  const quarter = Math.round(W / 4);
  render.drawText(batStr,  statusFont, batCol,
    Math.round(quarter     - batW  / 2), STATUS_Y);
  render.drawText(connStr, statusFont, connCol,
    Math.round(3 * quarter - connW / 2), STATUS_Y);

  // 10. Battery bar — always on top
  drawBatteryBar(pct, charging);

  render.end();

  frame = (frame + 1) % 1000;
}

/* ─── Events ─────────────────────────────────────────────────────────────── */
watch.addEventListener("minutechange",              draw);
watch.addEventListener("wake",                      draw);
watch.addEventListener("batterychange",             draw);
watch.addEventListener("chargingchange",            draw);
watch.addEventListener("bluetoothconnectionchange", draw);

draw();
