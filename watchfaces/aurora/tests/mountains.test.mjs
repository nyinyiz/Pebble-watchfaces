import test from 'node:test';
import assert from 'node:assert/strict';

import { buildMountainProfile } from '../src/embeddedjs/mountains.mjs';

test('buildMountainProfile returns null for round displays', () => {
  const profile = buildMountainProfile({
    width: 260,
    isRound: true,
    peaks: [[50, 14, 24]]
  });

  assert.equal(profile, null);
});

test('buildMountainProfile uses compact storage for rectangular displays', () => {
  const profile = buildMountainProfile({
    width: 200,
    isRound: false,
    peaks: [[20, 12, 20], [102, 17, 26], [185, 8, 14]]
  });

  assert.ok(profile instanceof Uint8Array);
  assert.equal(profile.length, 200);
  assert.equal(profile[0], 3);
  assert.ok(profile[20] > profile[0]);
  assert.ok(profile[102] >= profile[20]);
});
