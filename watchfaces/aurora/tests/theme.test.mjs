import test from 'node:test';
import assert from 'node:assert/strict';

import { getThemeForHour, getThemeNameForHour } from '../src/embeddedjs/theme.mjs';

test('getThemeNameForHour splits the day into morning, day, evening, and night', () => {
  assert.equal(getThemeNameForHour(5), 'night');
  assert.equal(getThemeNameForHour(6), 'morning');
  assert.equal(getThemeNameForHour(11), 'morning');
  assert.equal(getThemeNameForHour(12), 'day');
  assert.equal(getThemeNameForHour(16), 'day');
  assert.equal(getThemeNameForHour(17), 'evening');
  assert.equal(getThemeNameForHour(19), 'evening');
  assert.equal(getThemeNameForHour(20), 'night');
});

test('getThemeForHour returns distinct aurora palettes for each state', () => {
  const morning = getThemeForHour(8);
  const day = getThemeForHour(13);
  const evening = getThemeForHour(18);
  const night = getThemeForHour(23);

  assert.equal(morning.aurora.length, 3);
  assert.equal(day.aurora.length, 3);
  assert.equal(evening.aurora.length, 3);
  assert.equal(night.aurora.length, 3);

  assert.notDeepEqual(morning.accent, day.accent);
  assert.notDeepEqual(day.accent, evening.accent);
  assert.notDeepEqual(evening.accent, night.accent);
  assert.notDeepEqual(morning.aurora, evening.aurora);
  assert.notDeepEqual(day.aurora, night.aurora);
});
