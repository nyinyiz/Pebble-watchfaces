import { strict as assert } from 'node:assert';
import { test } from 'node:test';
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const __dir = dirname(fileURLToPath(import.meta.url));
const packageJson = JSON.parse(
  readFileSync(join(__dir, '../package.json'), 'utf8')
);
const mainSource = readFileSync(join(__dir, '../src/c/main.c'), 'utf8');

test('package targets every requested Pebble platform family', () => {
  assert.deepEqual(packageJson.pebble.targetPlatforms, [
    'aplite',
    'basalt',
    'chalk',
    'diorite',
    'emery',
    'flint',
    'gabbro',
  ]);
});

test('watchface source exposes a persisted style preference', () => {
  assert.match(mainSource, /PREF_KEY_STYLE/);
  assert.match(mainSource, /FACE_STYLE_DIGITAL/);
  assert.match(mainSource, /FACE_STYLE_ANALOG/);
  assert.match(mainSource, /persist_write_int\(PREF_KEY_STYLE/);
});

test('watchface source contains analog dial drawing hooks', () => {
  assert.match(mainSource, /graphics_draw_circle|graphics_context_set_stroke_width/);
});

test('watchface preference changes are driven by accelerometer taps, not buttons', () => {
  assert.match(mainSource, /accel_tap_service_subscribe/);
  assert.doesNotMatch(mainSource, /window_single_click_subscribe\(BUTTON_ID_SELECT/);
  assert.doesNotMatch(mainSource, /window_long_click_subscribe\(BUTTON_ID_SELECT/);
});
