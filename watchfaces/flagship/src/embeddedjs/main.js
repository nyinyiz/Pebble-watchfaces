import Poco from "commodetto/Poco";

const render = new Poco(screen);

/* ─── Fonts (names/sizes from the Pebble system font registry) ──────────────
 *   "Leco-Regular",  42  →  FONT_KEY_LECO_42_NUMBERS     (LCD-style digits)
 *   "Gothic-Bold",   24  →  FONT_KEY_GOTHIC_24_BOLD       (full char set)
 *   "Gothic-Bold",   18  →  FONT_KEY_GOTHIC_18_BOLD       (compact labels)
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

// Aurora bands — drawn back-to-front: violet → teal → vivid green
const C_AURORA = [
  render.makeColor(98,  58,  205),   // deep violet  (back)
  render.makeColor(50,  175, 200),   // teal          (mid)
  render.makeColor(50,  225, 115),   // vivid green   (front)
];

// Time-of-day accent colours
const C_DAWN  = render.makeColor(248, 184, 79);   // warm amber  (06:00–11:59)
const C_DAY   = render.makeColor(95,  208, 255);  // sky blue    (12:00–17:59)
const C_NIGHT = render.makeColor(128, 168, 245);  // cool violet (18:00–05:59)

/* ─── Screen geometry ────────────────────────────────────────────────────── */
const W        = render.width;
const H        = render.height;
const IS_ROUND = W === H;

/* ─── Star field ─────────────────────────────────────────────────────────── */
// Positions spread across aurora zone (Y 5–85 rect / 5–95 round).
// Larger entries (i % 4 === 1) are drawn 2×2 for a subtle magnitude variation.
const STARS_RECT = [
  [  8, 14], [ 25,  8], [ 50, 20], [ 75,  6], [ 98, 16],
  [125,  9], [150, 22], [178, 11], [194, 18],
  [ 15, 32], [ 40, 38], [ 70, 28], [100, 35], [128, 42],
  [155, 30], [184, 37],
  [ 10, 55], [ 45, 62], [ 85, 50], [115, 58], [148, 65],
  [180, 52],
  [ 30, 75], [110, 70],
];

// Scale x-positions for the wider round display
const STARS = IS_ROUND
  ? STARS_RECT.map(([x, y]) => [Math.round(x * W / 200), y + 5])
  : STARS_RECT;

/* ─── Aurora band configuration ──────────────────────────────────────────── */
// Rendered back-to-front so violet is at the bottom, green at the top.
//   baseY  — vertical centre of the sine wave on emery / gabbro
//   amp    — peak vertical displacement in pixels
//   halfH  — half the band thickness at its widest point
//   freq   — spatial frequency in radians per pixel
//   speed  — phase advance in radians per animation frame (= 1 per minute)
const AURORA_TOP = IS_ROUND ? 8  : 5;
const AURORA_BOT = IS_ROUND ? 95 : 85;

const BANDS = [
  { baseY: IS_ROUND ? 78 : 68, amp: 7,  halfH: 5, freq: 0.035, speed: 0.110 },
  { baseY: IS_ROUND ? 58 : 48, amp: 10, halfH: 7, freq: 0.025, speed: 0.165 },
  { baseY: IS_ROUND ? 35 : 28, amp: 9,  halfH: 5, freq: 0.018, speed: 0.140 },
];

/* ─── Animation frame counter (increments every minute) ─────────────────── */
let frame = 0;

/* ─── Layout Y positions ─────────────────────────────────────────────────── */
const DIVIDER_Y = IS_ROUND ? 100 : 88;
const TIME_Y    = IS_ROUND ? 108 : 96;   // Leco 42 → bottom ≈ TIME_Y + 42
const DATE_Y    = IS_ROUND ? 162 : 150;  // Gothic 24
const STATUS_Y  = IS_ROUND ? 204 : 192;  // Gothic 18

/* ─── Pure helpers ───────────────────────────────────────────────────────── */
function accentFor(hours) {
  if (hours < 6 || hours >= 18) return C_NIGHT;
  if (hours < 12) return C_DAWN;
  return C_DAY;
}

function pad2(n) { return n < 10 ? `0${n}` : `${n}`; }

/* ─── Star renderer ──────────────────────────────────────────────────────── */
function drawStars() {
  for (let i = 0; i < STARS.length; i++) {
    const [x, y] = STARS[i];
    // Rotate which stars appear "dimmed" each minute → subtle twinkle effect.
    const dimmed = (i + frame) % 5 === 0;
    const col    = dimmed ? C_AURORA[1] : C_WHITE;
    // Larger stars drawn 2×2 for magnitude variation.
    const sz     = (!dimmed && i % 4 === 1) ? 2 : 1;
    render.fillRectangle(col, x, y, sz, sz);
  }
}

/* ─── Aurora renderer ────────────────────────────────────────────────────── */
// For each x column, compute the sine-wave centre and draw a vertical stripe.
// Edge-fade: half-height tapers to zero within 15 px of each screen edge,
// giving natural-looking band termination rather than a hard cutoff.
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

/* ─── Battery bar renderer ───────────────────────────────────────────────── */
function drawBatteryBar(pct, charging) {
  const barW = Math.max(2, Math.round(W * pct / 100));
  const col  = charging    ? C_AURORA[1]
             : pct > 30    ? C_AURORA[2]
             : pct > 15    ? C_DAWN
             :               C_DANGER;
  render.fillRectangle(C_BATT_RAIL, 0, 0, W, 3);
  render.fillRectangle(col,         0, 0, barW, 3);
}

/* ─── Main render function ───────────────────────────────────────────────── */
function draw() {
  const now      = new Date();
  const hrs      = now.getHours();
  const acc      = accentFor(hrs);
  const pct      = typeof watch?.battery  === "number"  ? watch.battery  : 100;
  const charging = typeof watch?.charging === "boolean" ? watch.charging : false;
  const is24h    = watch?.timeStyle !== "12h";
  const connected = watch?.connected !== false;

  /* — Build display strings — */
  const h12      = hrs % 12 || 12;
  const minStr   = pad2(now.getMinutes());
  const timeStr  = is24h ? `${hrs}:${minStr}` : `${h12}:${minStr}`;
  const meridStr = is24h ? "" : hrs >= 12 ? "PM" : "AM";

  const DAYS   = ["SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"];
  const MONTHS = ["JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"];
  const dateStr = `${DAYS[now.getDay()]}  ${now.getDate()}  ${MONTHS[now.getMonth()]}`;

  const batStr  = charging    ? `${Math.round(pct)}% +`
                : pct <= 20   ? `${Math.round(pct)}% !`
                :               `${Math.round(pct)}%`;
  const connStr = connected ? "ONLINE" : "OFFLINE";

  /* — Render pass — */
  render.begin();

  // 1. Solid black background
  render.fillRectangle(C_BG, 0, 0, W, H);

  // 2. Star field (beneath aurora so they peek through the gaps)
  drawStars();

  // 3. Aurora bands — violet, then teal, then green on top
  drawAurora();

  // 4. Accent divider line — visually separates sky from dial area
  const divInset = IS_ROUND ? 40 : 12;
  render.drawLine(divInset, DIVIDER_Y, W - divInset, DIVIDER_Y, acc, 1);

  // 5. Time — LCD-style digits, white, horizontally centred
  const timeW = render.getTextWidth(timeStr, timeFont);
  const timeX = Math.round((W - timeW) / 2);
  render.drawText(timeStr, timeFont, C_WHITE, timeX, TIME_Y);

  // 6. Meridiem — accent colour, tucked to the right of the time digits
  if (meridStr) {
    const mW = render.getTextWidth(meridStr, dateFont);
    // Offset down so it sits at the lower baseline of the larger time glyph
    const mX = timeX + timeW + 5;
    const mY = TIME_Y + 22;
    if (mX + mW < W - 2) {
      render.drawText(meridStr, dateFont, acc, mX, mY);
    }
  }

  // 7. Date — accent colour, centred
  const dateW = render.getTextWidth(dateStr, dateFont);
  render.drawText(dateStr, dateFont, acc, Math.round((W - dateW) / 2), DATE_Y);

  // 8. Status row — battery on the left quarter, connection on the right quarter
  const batW    = render.getTextWidth(batStr,  statusFont);
  const connW   = render.getTextWidth(connStr, statusFont);
  const batCol  = pct <= 20  ? C_DANGER : C_DIM;
  const connCol = connected  ? C_DIM    : C_DANGER;
  const quarter = Math.round(W / 4);
  render.drawText(batStr,  statusFont, batCol,
    Math.round(quarter     - batW  / 2), STATUS_Y);
  render.drawText(connStr, statusFont, connCol,
    Math.round(3 * quarter - connW / 2), STATUS_Y);

  // 9. Battery fill bar — always drawn last so it is never occluded
  drawBatteryBar(pct, charging);

  render.end();

  // Advance frame counter (bounded to prevent integer overflow)
  frame = (frame + 1) % 1000;
}

/* ─── Event listeners ────────────────────────────────────────────────────── */
watch.addEventListener("minutechange",              draw);
watch.addEventListener("wake",                      draw);
watch.addEventListener("batterychange",             draw);
watch.addEventListener("chargingchange",            draw);
watch.addEventListener("bluetoothconnectionchange", draw);

draw();
