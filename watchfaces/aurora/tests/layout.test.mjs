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
  assert.deepEqual(layout.statusCenter, {
    x: 100,
    y: 176
  });
});

test('buildLayout fits small-rect screens (144×168: basalt, diorite)', () => {
  const layout = buildLayout({ width: 144, height: 168, isRound: false });

  assert.deepEqual(layout.time,         { x: 72,  y: 76  });
  assert.deepEqual(layout.date,         { x: 72,  y: 106 });
  assert.deepEqual(layout.statusLeft,   { x: 24,  y: 144 });
  assert.deepEqual(layout.statusCenter, { x: 72,  y: 144 });
  assert.deepEqual(layout.statusRight,  { x: 120, y: 144 });
  assert.deepEqual(layout.label,        { x: 72,  y: 30  });
});

test('buildLayout fits small-round screens (180×180: chalk)', () => {
  const layout = buildLayout({ width: 180, height: 180, isRound: true });

  assert.deepEqual(layout.time,         { x: 90,  y: 80  });
  assert.deepEqual(layout.date,         { x: 90,  y: 112 });
  assert.deepEqual(layout.statusLeft,   { x: 30,  y: 140 });
  assert.deepEqual(layout.statusCenter, { x: 90,  y: 140 });
  assert.deepEqual(layout.statusRight,  { x: 150, y: 140 });
  assert.deepEqual(layout.label,        { x: 90,  y: 22  });
});

test('buildLayout moves content inward on round screens', () => {
  const layout = buildLayout({ width: 260, height: 260, isRound: true });

  assert.deepEqual(layout.time, {
    x: 130,
    y: 110
  });
  assert.deepEqual(layout.meridiem, {
    x: 130,
    y: 54
  });
  assert.deepEqual(layout.statusLeft, {
    x: 44,
    y: 192
  });
  assert.deepEqual(layout.statusCenter, {
    x: 130,
    y: 192
  });
  assert.deepEqual(layout.statusRight, {
    x: 216,
    y: 192
  });
  assert.deepEqual(layout.label, {
    x: 130,
    y: 30
  });
});
