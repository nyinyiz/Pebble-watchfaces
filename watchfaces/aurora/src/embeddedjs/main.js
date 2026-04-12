import Poco from "commodetto/Poco";
import Battery from "embedded:sensor/Battery";
import { buildAnimationState } from "animation";
import { buildMountainProfile } from "mountains";

const render = new Poco(screen);

const timeFont = new render.Font("Leco-Regular", 42);
const dateFont = new render.Font("Gothic-Bold", 24);
const statusFont = new render.Font("Gothic-Bold", 18);

const C_BG = render.makeColor(0, 0, 0);
const C_WHITE = render.makeColor(255, 255, 255);
const C_DIM = render.makeColor(100, 106, 128);
const C_BATT_RAIL = render.makeColor(22, 22, 32);
const C_DANGER = render.makeColor(235, 87, 87);
const C_SNOW = render.makeColor(210, 230, 255);

const C_AURORA = [
  render.makeColor(98, 58, 205),
  render.makeColor(50, 175, 200),
  render.makeColor(50, 225, 115),
];

const C_DAWN = render.makeColor(248, 184, 79);
const C_DAY = render.makeColor(95, 208, 255);
const C_NIGHT = render.makeColor(128, 168, 245);

const W = render.width;
const H = render.height;
const IS_ROUND = W === H;

const STARS_RECT = [
  [8, 12], [25, 7], [50, 18], [75, 5], [98, 14],
  [125, 8], [150, 20], [178, 10], [194, 17],
  [15, 30], [40, 36], [70, 26], [100, 33], [128, 40],
  [155, 28], [184, 35],
  [10, 52], [45, 58], [85, 48], [115, 55], [148, 62],
  [180, 50],
];

const STARS = IS_ROUND
  ? STARS_RECT.map(([x, y]) => [Math.round((x * W) / 200), y + 5])
  : STARS_RECT;

const AURORA_TOP = IS_ROUND ? 8 : 5;
const AURORA_BOT = IS_ROUND ? 95 : 85;

const BANDS = [
  { baseY: IS_ROUND ? 78 : 68, amp: 7, halfH: 5, freq: 0.035 },
  { baseY: IS_ROUND ? 58 : 48, amp: 10, halfH: 7, freq: 0.025 },
  { baseY: IS_ROUND ? 35 : 28, amp: 9, halfH: 5, freq: 0.018 },
];

const MTN_PEAKS = IS_ROUND
  ? [[50, 14, 24], [95, 12, 18], [132, 20, 30], [180, 13, 20], [230, 10, 16]]
  : [[20, 12, 20], [58, 10, 16], [102, 17, 26], [148, 11, 18], [185, 8, 14]];

const MTN_BASE = AURORA_BOT;
const MTN_PROFILE = buildMountainProfile({
  width: W,
  isRound: IS_ROUND,
  peaks: MTN_PEAKS
});

const DIVIDER_Y = IS_ROUND ? 100 : 88;
const LAYOUT = buildLayout({ width: W, height: H, isRound: IS_ROUND });

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

function drawStars(animation) {
  for (let i = 0; i < STARS.length; i++) {
    const [x, y] = STARS[i];
    const dimmed = (i + animation.twinkleOffset) % 5 === 0;
    const color = dimmed ? C_AURORA[1] : C_WHITE;
    const size = (!dimmed && i % 4 === 1) ? 2 : 1;
    render.fillRectangle(color, x, y, size, size);
  }
}

function drawAurora(animation) {
  const step = 2;

  for (let bandIndex = 0; bandIndex < BANDS.length; bandIndex++) {
    const { baseY, amp, halfH, freq } = BANDS[bandIndex];
    const phase = animation.bandPhases[bandIndex];
    const animatedHalfH = halfH + (bandIndex === 1 ? Math.round(animation.pulse * 2) : 0);

    for (let x = 0; x < W; x += step) {
      const edge = x < 15 ? x : (W - 1 - x < 15 ? W - 1 - x : 15);
      const columnHalfHeight = Math.round((animatedHalfH * edge) / 15);
      if (columnHalfHeight <= 0) {
        continue;
      }

      const centerY = Math.round(baseY + Math.sin((x * freq) + phase) * amp);
      const topY = Math.max(centerY - columnHalfHeight, AURORA_TOP);
      const bottomY = Math.min(centerY + columnHalfHeight, AURORA_BOT);
      if (bottomY > topY) {
        render.fillRectangle(C_AURORA[bandIndex], x, topY, Math.min(step, W - x), bottomY - topY);
      }
    }
  }
}

function drawSecondSweep(animation, accentColor) {
  const pulseLeft = Math.max(
    animation.sweepInset,
    animation.sweepX - animation.sweepHalfWidth
  );
  const pulseRight = Math.min(
    W - animation.sweepInset,
    animation.sweepX + animation.sweepHalfWidth
  );
  const pulseWidth = Math.max(1, pulseRight - pulseLeft);

  render.fillRectangle(accentColor, pulseLeft, DIVIDER_Y - 1, pulseWidth, 3);

  if (pulseLeft > animation.sweepInset) {
    render.drawLine(animation.sweepInset, DIVIDER_Y, pulseLeft, DIVIDER_Y, C_DIM, 1);
  }
}

function drawMountains() {
  if (!MTN_PROFILE) {
    render.fillRectangle(C_BG, 0, MTN_BASE - 4, W, DIVIDER_Y - MTN_BASE + 4);
    return;
  }

  for (let x = 0; x < W; x += 2) {
    const height = Math.max(MTN_PROFILE[x], MTN_PROFILE[x + 1] || 0);
    render.fillRectangle(C_BG, x, MTN_BASE - height, Math.min(2, W - x), DIVIDER_Y - MTN_BASE + height);
  }

  let previousHeight = MTN_PROFILE[0];
  for (let x = 1; x < W - 1; x++) {
    const height = MTN_PROFILE[x];
    if (height > previousHeight && height >= MTN_PROFILE[x + 1] && height >= 9) {
      render.fillRectangle(C_SNOW, x - 1, MTN_BASE - height - 1, 3, 2);
    }
    previousHeight = height;
  }
}

function drawBatteryBar(percent, charging) {
  const barWidth = Math.max(2, Math.round((W * percent) / 100));
  const color = charging ? C_AURORA[1]
    : percent > 30 ? C_AURORA[2]
    : percent > 15 ? C_DAWN
    : C_DANGER;
  render.fillRectangle(C_BATT_RAIL, 0, 0, W, 3);
  render.fillRectangle(color, 0, 0, barWidth, 3);
}

let batteryPercent = 100;
let isCharging = false;

const batteryMonitor = new Battery({
  onSample() {
    const sample = batteryMonitor.sample();
    batteryPercent = sample.percent;
    isCharging = sample.charging;
    draw({ date: new Date() });
  }
});

{
  const sample = batteryMonitor.sample();
  batteryPercent = sample.percent;
  isCharging = sample.charging;
}

function draw(event) {
  const now = event?.date ?? new Date();
  const animation = buildAnimationState({ date: now, width: W, isRound: IS_ROUND });
  const is24Hour = !watch.hour12;
  const isConnected = watch.connected?.app ?? true;
  const model = buildFaceModel({
    date: now,
    is24Hour,
    batteryPercent,
    isCharging,
    isConnected
  });
  const accentColor = accentFor(now.getHours());

  const timeText = model.time.text;
  const meridiemText = model.time.meridiem;
  const dateText = model.date.text;
  const batteryText = model.statusLeft.text;
  const secondsText = model.statusCenter.text;
  const connectionText = model.statusRight.text;

  render.begin();
  render.fillRectangle(C_BG, 0, 0, W, H);

  drawStars(animation);
  drawAurora(animation);
  drawMountains();

  render.drawLine(animation.sweepInset, DIVIDER_Y, W - animation.sweepInset, DIVIDER_Y, accentColor, 1);
  drawSecondSweep(animation, accentColor);

  const timeWidth = render.getTextWidth(timeText, timeFont);
  const timeX = Math.round(LAYOUT.time.x - (timeWidth / 2));
  render.drawText(timeText, timeFont, C_WHITE, timeX, LAYOUT.time.y);

  if (meridiemText) {
    const meridiemWidth = render.getTextWidth(meridiemText, dateFont);
    const meridiemX = timeX + timeWidth + 5;
    const meridiemY = LAYOUT.time.y + 22;
    if (meridiemX + meridiemWidth < W - 2) {
      render.drawText(meridiemText, dateFont, accentColor, meridiemX, meridiemY);
    }
  }

  const dateWidth = render.getTextWidth(dateText, dateFont);
  render.drawText(dateText, dateFont, accentColor,
    Math.round(LAYOUT.date.x - (dateWidth / 2)), LAYOUT.date.y);

  const batteryWidth = render.getTextWidth(batteryText, statusFont);
  const secondsWidth = render.getTextWidth(secondsText, statusFont);
  const connectionWidth = render.getTextWidth(connectionText, statusFont);
  const batteryColor = batteryPercent <= 20 ? C_DANGER : C_DIM;
  const connectionColor = isConnected ? C_DIM : C_DANGER;
  render.drawText(batteryText, statusFont, batteryColor,
    Math.round(LAYOUT.statusLeft.x - (batteryWidth / 2)), LAYOUT.statusLeft.y);
  render.drawText(secondsText, statusFont, accentColor,
    Math.round(LAYOUT.statusCenter.x - (secondsWidth / 2)), LAYOUT.statusCenter.y);
  render.drawText(connectionText, statusFont, connectionColor,
    Math.round(LAYOUT.statusRight.x - (connectionWidth / 2)), LAYOUT.statusRight.y);

  drawBatteryBar(batteryPercent, isCharging);
  render.end();
}

watch.addEventListener("secondchange", draw);
watch.addEventListener("connected", () => draw({ date: new Date() }));

draw({ date: new Date() });
