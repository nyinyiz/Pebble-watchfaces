const { formatBattery, formatConnection, formatDate, formatTime } = require('./lib/format');
const { buildLayout } = require('./lib/layout');

function getBatteryPercent() {
  if (typeof navigator !== 'undefined' && navigator.battery && typeof navigator.battery.level === 'number') {
    return navigator.battery.level * 100;
  }

  return 100;
}

function getConnectionLabel() {
  if (typeof Pebble !== 'undefined' && Pebble.getActiveWatchInfo) {
    return formatConnection(true);
  }

  return formatConnection(false);
}

function drawFace(ctx, bounds) {
  const isRound = typeof rocky !== 'undefined' && typeof rocky.on === 'function' ? rocky.round : false;
  const layout = buildLayout({
    width: bounds.w,
    height: bounds.h,
    isRound
  });
  const now = new Date();
  const is24Hour = typeof clock !== 'undefined' && clock.is24HourStyle ? clock.is24HourStyle() : true;

  ctx.clearRect(0, 0, bounds.w, bounds.h);
  ctx.fillStyle = 'black';
  ctx.fillRect(0, 0, bounds.w, bounds.h);

  ctx.textAlign = 'center';
  ctx.fillStyle = 'white';

  ctx.font = '42px Gothic';
  ctx.fillText(formatTime(now, is24Hour), layout.time.x, layout.time.y, bounds.w - 24);

  ctx.font = '18px Gothic';
  ctx.fillStyle = '#AAAAAA';
  ctx.fillText(formatDate(now), layout.date.x, layout.date.y, bounds.w - 32);

  ctx.font = '16px Gothic';
  ctx.fillStyle = '#7FDBFF';
  ctx.fillText(formatBattery(getBatteryPercent()), layout.statusLeft.x, layout.statusLeft.y, 60);

  ctx.fillStyle = '#2ECC71';
  ctx.fillText(getConnectionLabel(), layout.statusRight.x, layout.statusRight.y, 100);
}

if (typeof rocky !== 'undefined' && rocky.on) {
  rocky.on('draw', (event) => {
    drawFace(event.context, event.bounds);
  });

  rocky.on('minutechange', () => {
    rocky.requestDraw();
  });
}

module.exports = {
  drawFace
};
