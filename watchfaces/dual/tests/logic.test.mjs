/**
 * Logic tests for the time calculation and formatting functions.
 * These mirror the C functions in main.c so we can test without an emulator.
 */
import { strict as assert } from 'node:assert';
import { test } from 'node:test';

// ── JS re-implementations of the C logic ──────────────────────────────────

/**
 * Mirror of format_offset() in main.c.
 * offset_minutes: signed integer, negative = west of UTC
 * Returns string like "UTC+5:30", "UTC-5", "UTC+0"
 */
function formatOffset(offsetMinutes) {
  const neg = offsetMinutes < 0;
  const absM = Math.abs(offsetMinutes);
  const h = Math.floor(absM / 60);
  const m = absM % 60;
  const sign = neg ? '-' : '+';
  return m === 0 ? `UTC${sign}${h}` : `UTC${sign}${h}:${String(m).padStart(2, '0')}`;
}

/**
 * Mirror of the second-timezone calculation in update_display().
 * utcNow: Unix timestamp (seconds since epoch, UTC)
 * offsetMinutes: signed integer
 * Returns { hours, minutes } of the other timezone
 */
function calcOtherTime(utcNow, offsetMinutes) {
  const otherEpoch = utcNow + offsetMinutes * 60;
  const d = new Date(otherEpoch * 1000);
  // gmtime equivalent: interpret the epoch as UTC
  return {
    hours: d.getUTCHours(),
    minutes: d.getUTCMinutes(),
  };
}

// ── format_offset tests ────────────────────────────────────────────────────

test('format_offset: UTC+0', () => {
  assert.equal(formatOffset(0), 'UTC+0');
});

test('format_offset: UTC+5:30 (India, 330 min)', () => {
  assert.equal(formatOffset(330), 'UTC+5:30');
});

test('format_offset: UTC+5:45 (Nepal, 345 min)', () => {
  assert.equal(formatOffset(345), 'UTC+5:45');
});

test('format_offset: UTC-5 (USA Eastern Standard, -300 min)', () => {
  assert.equal(formatOffset(-300), 'UTC-5');
});

test('format_offset: UTC-4:30 (-270 min)', () => {
  assert.equal(formatOffset(-270), 'UTC-4:30');
});

test('format_offset: UTC+14 (Kiribati Line, 840 min)', () => {
  assert.equal(formatOffset(840), 'UTC+14');
});

test('format_offset: UTC+9:30 (Australia Darwin, 570 min)', () => {
  assert.equal(formatOffset(570), 'UTC+9:30');
});

test('format_offset: UTC-3:30 (Canada Newfoundland, -210 min)', () => {
  assert.equal(formatOffset(-210), 'UTC-3:30');
});

test('format_offset: UTC+6:30 (Myanmar, 390 min)', () => {
  assert.equal(formatOffset(390), 'UTC+6:30');
});

// ── Time calculation tests ─────────────────────────────────────────────────

test('calcOtherTime: UTC 12:00 + UTC+5:30 = 17:30 (India)', () => {
  // 2026-01-01 12:00:00 UTC
  const utcNoon = Date.UTC(2026, 0, 1, 12, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utcNoon, 330);
  assert.equal(hours, 17);
  assert.equal(minutes, 30);
});

test('calcOtherTime: UTC 12:00 + UTC-5 = 07:00 (New York EST)', () => {
  const utcNoon = Date.UTC(2026, 0, 1, 12, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utcNoon, -300);
  assert.equal(hours, 7);
  assert.equal(minutes, 0);
});

test('calcOtherTime: UTC 00:00 + UTC+14 = 14:00 same day (Kiribati)', () => {
  const utcMidnight = Date.UTC(2026, 0, 1, 0, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utcMidnight, 840);
  assert.equal(hours, 14);
  assert.equal(minutes, 0);
});

test('calcOtherTime: UTC 23:00 + UTC-5 = 18:00 same day (New York EST)', () => {
  const utc11pm = Date.UTC(2026, 0, 1, 23, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utc11pm, -300);
  assert.equal(hours, 18);
  assert.equal(minutes, 0);
});

test('calcOtherTime: UTC 20:00 + UTC+5:45 = 01:45 next day (Nepal)', () => {
  const utc8pm = Date.UTC(2026, 0, 1, 20, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utc8pm, 345);
  // 20:00 + 5:45 = 25:45 → 01:45 next day
  assert.equal(hours, 1);
  assert.equal(minutes, 45);
});

test('calcOtherTime: UTC 02:00 + UTC-8 = 18:00 previous day (LA PST)', () => {
  const utc2am = Date.UTC(2026, 0, 1, 2, 0, 0) / 1000;
  const { hours, minutes } = calcOtherTime(utc2am, -480);
  // 02:00 - 8h = -6h → 18:00 previous day
  assert.equal(hours, 18);
  assert.equal(minutes, 0);
});

test('calcOtherTime: same offset as local = same time', () => {
  const nowEpoch = Date.UTC(2026, 3, 12, 10, 42, 0) / 1000;
  // UTC+0: local IS UTC, other at UTC+0 should also be same
  const { hours, minutes } = calcOtherTime(nowEpoch, 0);
  assert.equal(hours, 10);
  assert.equal(minutes, 42);
});
