import test from 'node:test';
import assert from 'node:assert/strict';

import { buildFaceModel } from '../src/common/model.mjs';

test('buildFaceModel derives a calm morning palette and 24-hour time', () => {
  const model = buildFaceModel({
    date: new Date('2026-04-11T09:05:00'),
    is24Hour: true,
    batteryPercent: 82,
    isCharging: false,
    isConnected: true
  });

  assert.equal(model.time.text, '9:05');
  assert.equal(model.time.meridiem, '');
  assert.equal(model.statusLeft.text, '82%');
  assert.equal(model.statusCenter.text, 'SEC 00');
  assert.equal(model.statusRight.text, 'BT OK');
  assert.equal(model.palette.accentName, 'morning');
});

test('buildFaceModel adds meridiem and charging text in 12-hour mode', () => {
  const model = buildFaceModel({
    date: new Date('2026-04-11T21:07:49'),
    is24Hour: false,
    batteryPercent: 58,
    isCharging: true,
    isConnected: false
  });

  assert.equal(model.time.text, '9:07');
  assert.equal(model.time.meridiem, 'PM');
  assert.equal(model.statusLeft.text, '58% CHG');
  assert.equal(model.statusCenter.text, 'SEC 49');
  assert.equal(model.statusRight.text, 'BT OFF');
  assert.equal(model.palette.accentName, 'night');
});

test('buildFaceModel surfaces a low-battery warning label', () => {
  const model = buildFaceModel({
    date: new Date('2026-04-11T14:15:00'),
    is24Hour: true,
    batteryPercent: 14,
    isCharging: false,
    isConnected: true
  });

  assert.equal(model.statusLeft.text, '14% LOW');
  assert.equal(model.statusCenter.text, 'SEC 00');
  assert.equal(model.statusRight.text, 'BT OK');
  assert.equal(model.palette.accentName, 'day');
});
