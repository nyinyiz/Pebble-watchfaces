/**
 * Timezone data integrity tests.
 * Mirrors the C struct so we can validate the data without an emulator.
 */
import { strict as assert } from 'node:assert';
import { test } from 'node:test';

// ── Re-import timezone data from a JS mirror ──────────────────────────────
// We parse timezones.h directly so the test is always in sync with the C source.
import { readFileSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const __dir = dirname(fileURLToPath(import.meta.url));
const hSource = readFileSync(join(__dir, '../src/c/timezones.h'), 'utf8');

// Parse all { "label", "city", offset } entries from the C header
const ENTRY_RE = /\{\s*"([^"]+)"\s*,\s*"([^"]+)"\s*,\s*(-?\d+)\s*\}/g;
const TIMEZONES = [];
for (const m of hSource.matchAll(ENTRY_RE)) {
  TIMEZONES.push({ label: m[1], city: m[2], offset: parseInt(m[3], 10) });
}

// ── Tests ─────────────────────────────────────────────────────────────────

test('timezone table is non-empty', () => {
  assert.ok(TIMEZONES.length > 0, 'No entries parsed');
  console.log(`  Parsed ${TIMEZONES.length} timezone entries`);
});

test('all offsets are in valid range [-720, 840]', () => {
  for (const tz of TIMEZONES) {
    assert.ok(
      tz.offset >= -720 && tz.offset <= 840,
      `${tz.label}: offset ${tz.offset} out of range`
    );
  }
});

test('all labels are non-empty strings', () => {
  for (const tz of TIMEZONES) {
    assert.ok(tz.label.length > 0, 'Empty label found');
    assert.ok(tz.label.length <= 32, `Label too long: "${tz.label}"`);
  }
});

test('all city names are non-empty strings', () => {
  for (const tz of TIMEZONES) {
    assert.ok(tz.city.length > 0, 'Empty city found');
    assert.ok(tz.city.length <= 20, `City too long: "${tz.city}"`);
  }
});

test('no duplicate labels', () => {
  const seen = new Set();
  for (const tz of TIMEZONES) {
    assert.ok(!seen.has(tz.label), `Duplicate label: "${tz.label}"`);
    seen.add(tz.label);
  }
});

test('UTC entry exists at offset 0', () => {
  const utc = TIMEZONES.find(tz => tz.label === 'UTC');
  assert.ok(utc, 'No UTC entry found');
  assert.equal(utc.offset, 0);
});

test('India is at UTC+5:30 (330 minutes)', () => {
  const india = TIMEZONES.find(tz => tz.label === 'India');
  assert.ok(india, 'No India entry');
  assert.equal(india.offset, 330);
  assert.equal(india.city, 'Mumbai');
});

test('USA Eastern Standard is at UTC-5 (-300 minutes)', () => {
  const est = TIMEZONES.find(tz => tz.label === 'USA - Eastern (Standard)');
  assert.ok(est, 'No USA Eastern Standard entry');
  assert.equal(est.offset, -300);
});

test('USA Eastern Summer/DST is at UTC-4 (-240 minutes)', () => {
  const edt = TIMEZONES.find(tz => tz.label === 'USA - Eastern (Summer/DST)');
  assert.ok(edt, 'No USA Eastern Summer entry');
  assert.equal(edt.offset, -240);
});

test('Nepal is at UTC+5:45 (345 minutes)', () => {
  const nepal = TIMEZONES.find(tz => tz.label === 'Nepal');
  assert.ok(nepal, 'No Nepal entry');
  assert.equal(nepal.offset, 345);
});

test('Kiribati Line Islands is at UTC+14 (840 minutes) — world maximum', () => {
  const line = TIMEZONES.find(tz => tz.label === 'Kiribati - Line');
  assert.ok(line, 'No Kiribati Line entry');
  assert.equal(line.offset, 840);
});

test('Baker Island would be UTC-12 — minimum represented offset is >= -720', () => {
  const min = Math.min(...TIMEZONES.map(tz => tz.offset));
  assert.ok(min >= -720, `Minimum offset ${min} is below -720`);
});

test('labels are sorted A-Z (case-insensitive)', () => {
  for (let i = 1; i < TIMEZONES.length; i++) {
    const prev = TIMEZONES[i - 1].label.toLowerCase();
    const curr = TIMEZONES[i].label.toLowerCase();
    assert.ok(
      prev <= curr,
      `Out of order: "${TIMEZONES[i - 1].label}" before "${TIMEZONES[i].label}"`
    );
  }
});
