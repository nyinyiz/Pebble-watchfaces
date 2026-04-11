import test from 'node:test';
import assert from 'node:assert/strict';

import {
  formatTime,
  formatDate,
  formatBattery,
  formatConnection
} from '../src/common/format.mjs';

test('formatTime renders 24-hour time without leading hour zero', () => {
  const date = new Date('2026-04-11T09:05:00');

  assert.equal(formatTime(date, true), '9:05');
});

test('formatTime renders 12-hour time and strips midnight hour zero', () => {
  const date = new Date('2026-04-11T00:07:00');

  assert.equal(formatTime(date, false), '12:07');
});

test('formatDate renders compact uppercase date text for the face', () => {
  const date = new Date('2026-04-11T09:05:00');

  assert.equal(formatDate(date), 'SAT 11 APR');
});

test('formatBattery rounds percentage for compact display', () => {
  assert.equal(formatBattery(57.6), '58%');
});

test('formatConnection maps bluetooth state to face label', () => {
  assert.equal(formatConnection(true), 'BT OK');
  assert.equal(formatConnection(false), 'BT OFF');
});
