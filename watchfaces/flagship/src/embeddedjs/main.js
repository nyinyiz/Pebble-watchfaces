import Poco from "commodetto/Poco";

import { formatBattery, formatConnection, formatDate, formatTime } from "../common/format.mjs";
import { buildLayout } from "../common/layout.mjs";

const render = new Poco(screen);
const timeFont = new render.Font("Bitham-Black", 30);
const metaFont = new render.Font("Gothic", 18);
const statusFont = new render.Font("Gothic", 14);

const white = render.makeColor(255, 255, 255);
const black = render.makeColor(0, 0, 0);
const muted = render.makeColor(170, 170, 170);
const blue = render.makeColor(127, 219, 255);
const green = render.makeColor(46, 204, 113);

function readBatteryPercent() {
  if (globalThis.watch?.battery) {
    return globalThis.watch.battery;
  }

  return 100;
}

function readConnectionState() {
  if (globalThis.watch?.connected === false) {
    return false;
  }

  return true;
}

function draw() {
  const bounds = {
    width: render.width,
    height: render.height,
    isRound: render.width === render.height
  };
  const layout = buildLayout(bounds);
  const now = new Date();
  const timeText = formatTime(now, true);
  const dateText = formatDate(now);
  const batteryText = formatBattery(readBatteryPercent());
  const connectionText = formatConnection(readConnectionState());
  const timeWidth = render.getTextWidth(timeText, timeFont);
  const dateWidth = render.getTextWidth(dateText, metaFont);
  const batteryWidth = render.getTextWidth(batteryText, statusFont);
  const connectionWidth = render.getTextWidth(connectionText, statusFont);

  render.begin();
  render.fillRectangle(black, 0, 0, render.width, render.height);

  render.drawText(
    timeText,
    timeFont,
    white,
    Math.round((render.width - timeWidth) / 2),
    layout.time.y - Math.round(timeFont.height / 2)
  );

  render.drawText(
    dateText,
    metaFont,
    muted,
    Math.round((render.width - dateWidth) / 2),
    layout.date.y - Math.round(metaFont.height / 2)
  );

  render.drawText(
    batteryText,
    statusFont,
    blue,
    layout.statusLeft.x - Math.round(batteryWidth / 2),
    layout.statusLeft.y - Math.round(statusFont.height / 2)
  );

  render.drawText(
    connectionText,
    statusFont,
    green,
    layout.statusRight.x - Math.round(connectionWidth / 2),
    layout.statusRight.y - Math.round(statusFont.height / 2)
  );

  render.end();
}

watch.addEventListener("minutechange", draw);
watch.addEventListener("wake", draw);

draw();
