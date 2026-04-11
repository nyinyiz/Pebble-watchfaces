const TWO_PI = Math.PI * 2;
const BAND_MULTIPLIERS = [0.85, 1.25, 1.65];
const BAND_OFFSETS = [0.2, 1.1, 2.4];

export function buildAnimationState({ date, width, isRound }) {
  const seconds = date.getSeconds();
  const milliseconds = date.getMilliseconds?.() ?? 0;
  const secondProgress = ((seconds * 1000) + milliseconds) / 60000;
  const sweepInset = isRound ? 40 : 12;
  const sweepHalfWidth = isRound ? 9 : 7;
  const trackWidth = Math.max(0, width - (sweepInset * 2));
  const sweepX = sweepInset + Math.round(trackWidth * secondProgress);
  const bandPhases = BAND_MULTIPLIERS.map((multiplier, index) => {
    const raw = (secondProgress * TWO_PI * multiplier) + BAND_OFFSETS[index];
    return raw % TWO_PI;
  });

  return {
    secondProgress,
    sweepInset,
    sweepHalfWidth,
    sweepX,
    bandPhases,
    twinkleOffset: seconds % 5,
    pulse: Math.sin(secondProgress * TWO_PI)
  };
}
