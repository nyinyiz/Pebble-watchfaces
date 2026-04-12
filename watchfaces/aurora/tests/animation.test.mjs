import test from 'node:test';
import assert from 'node:assert/strict';

import { buildAnimationState } from '../src/common/animation.mjs';

test('buildAnimationState maps seconds to a left-to-right sweep on rectangular screens', () => {
  const state = buildAnimationState({
    date: new Date('2026-04-11T17:38:15.000'),
    width: 200,
    isRound: false
  });

  assert.equal(state.secondProgress, 0.25);
  assert.equal(state.sweepInset, 12);
  assert.equal(state.sweepHalfWidth, 7);
  assert.equal(state.sweepX, 56);
  assert.equal(state.twinkleOffset, 0);
});

test('buildAnimationState derives distinct band phases and a centered round sweep', () => {
  const state = buildAnimationState({
    date: new Date('2026-04-11T17:38:30.000'),
    width: 180,
    isRound: true
  });

  assert.equal(state.secondProgress, 0.5);
  assert.equal(state.sweepInset, 40);
  assert.equal(state.sweepHalfWidth, 9);
  assert.equal(state.sweepX, 90);
  assert.deepEqual(
    state.bandPhases.map((value) => Number(value.toFixed(3))),
    [2.87, 5.027, 1.3]
  );
});
