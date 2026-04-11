import test from 'node:test';
import assert from 'node:assert/strict';

import { buildLayout } from '../src/common/layout.mjs';

test('buildLayout centers the time block on rectangular screens', () => {
  const layout = buildLayout({ width: 200, height: 228, isRound: false });

  assert.deepEqual(layout.time, {
    x: 100,
    y: 96
  });
  assert.deepEqual(layout.meridiem, {
    x: 100,
    y: 70
  });
  assert.deepEqual(layout.date, {
    x: 100,
    y: 128
  });
  assert.deepEqual(layout.label, {
    x: 100,
    y: 42
  });
});

test('buildLayout moves content inward on round screens', () => {
  const layout = buildLayout({ width: 180, height: 180, isRound: true });

  assert.deepEqual(layout.time, {
    x: 90,
    y: 78
  });
  assert.deepEqual(layout.meridiem, {
    x: 90,
    y: 54
  });
  assert.deepEqual(layout.statusLeft, {
    x: 40,
    y: 128
  });
  assert.deepEqual(layout.statusRight, {
    x: 140,
    y: 128
  });
  assert.deepEqual(layout.label, {
    x: 90,
    y: 30
  });
});
