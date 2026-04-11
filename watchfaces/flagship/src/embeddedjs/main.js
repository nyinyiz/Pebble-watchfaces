import Poco from "commodetto/Poco";

import { buildLayout } from "../common/layout.mjs";
import { buildFaceModel } from "../common/model.mjs";

const render = new Poco(screen);
const labelFont = new render.Font("Gothic", 14);
const meridiemFont = new render.Font("Gothic", 16);
const timeFont = new render.Font("Bitham-Black", 36);
const metaFont = new render.Font("Gothic", 18);
const statusFont = new render.Font("Gothic", 14);

const white = render.makeColor(255, 255, 255);
const black = render.makeColor(0, 0, 0);
const muted = render.makeColor(170, 170, 170);
const dawn = render.makeColor(248, 184, 79);
const day = render.makeColor(127, 219, 255);
const night = render.makeColor(137, 180, 250);
const offline = render.makeColor(235, 87, 87);

const accentColors = {
  morning: dawn,
  day,
  night
};

function readBatteryPercent() {
  if (typeof globalThis.watch?.battery === "number") {
    return globalThis.watch.battery;
  }

  return 100;
}

function readChargingState() {
  if (typeof globalThis.watch?.charging === "boolean") {
    return globalThis.watch.charging;
  }

  return false;
}

function readConnectionState() {
  if (globalThis.watch?.connected === false) {
    return false;
  }

  return true;
}

function readTimeFormat() {
  if (globalThis.watch?.timeStyle === "12h") {
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
  const model = buildFaceModel({
    date: new Date(),
    is24Hour: readTimeFormat(),
    batteryPercent: readBatteryPercent(),
    isCharging: readChargingState(),
    isConnected: readConnectionState()
  });
  const accent = accentColors[model.palette.accentName] || day;
  const labelText = "FLAGSHIP";
  const labelWidth = render.getTextWidth(labelText, labelFont);
  const meridiemWidth = render.getTextWidth(model.time.meridiem, meridiemFont);
  const timeWidth = render.getTextWidth(model.time.text, timeFont);
  const dateWidth = render.getTextWidth(model.date.text, metaFont);
  const batteryWidth = render.getTextWidth(model.statusLeft.text, statusFont);
  const connectionWidth = render.getTextWidth(model.statusRight.text, statusFont);
  const connectionColor = model.statusRight.text === "OFFLINE" ? offline : accent;

  render.begin();
  render.fillRectangle(black, 0, 0, render.width, render.height);

  render.drawText(
    labelText,
    labelFont,
    accent,
    Math.round((render.width - labelWidth) / 2),
    layout.label.y - Math.round(labelFont.height / 2)
  );

  if (model.time.meridiem) {
    render.drawText(
      model.time.meridiem,
      meridiemFont,
      accent,
      Math.round((render.width - meridiemWidth) / 2),
      layout.meridiem.y - Math.round(meridiemFont.height / 2)
    );
  }

  render.drawText(
    model.time.text,
    timeFont,
    white,
    Math.round((render.width - timeWidth) / 2),
    layout.time.y - Math.round(timeFont.height / 2)
  );

  render.drawText(
    model.date.text,
    metaFont,
    muted,
    Math.round((render.width - dateWidth) / 2),
    layout.date.y - Math.round(metaFont.height / 2)
  );

  render.drawText(
    model.statusLeft.text,
    statusFont,
    accent,
    layout.statusLeft.x - Math.round(batteryWidth / 2),
    layout.statusLeft.y - Math.round(statusFont.height / 2)
  );

  render.drawText(
    model.statusRight.text,
    statusFont,
    connectionColor,
    layout.statusRight.x - Math.round(connectionWidth / 2),
    layout.statusRight.y - Math.round(statusFont.height / 2)
  );

  render.end();
}

watch.addEventListener("minutechange", draw);
watch.addEventListener("wake", draw);
watch.addEventListener("batterychange", draw);
watch.addEventListener("chargingchange", draw);
watch.addEventListener("bluetoothconnectionchange", draw);

draw();
