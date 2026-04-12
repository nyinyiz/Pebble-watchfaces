export function buildMountainProfile({ width, isRound, peaks, minimumHeight = 3 }) {
  if (isRound) {
    return null;
  }

  const profile = new Uint8Array(width);

  for (let x = 0; x < width; x++) {
    let height = minimumHeight;

    for (let i = 0; i < peaks.length; i++) {
      const [centerX, peakHeight, spread] = peaks[i];
      const distance = x - centerX;

      if (distance > -spread && distance < spread) {
        const candidate = Math.round(peakHeight * (1 - ((distance * distance) / (spread * spread))));
        if (candidate > height) {
          height = candidate;
        }
      }
    }

    profile[x] = height;
  }

  return profile;
}
