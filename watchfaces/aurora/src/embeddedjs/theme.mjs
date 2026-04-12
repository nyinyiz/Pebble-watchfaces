export const THEMES = {
  morning: {
    accent: [248, 184, 79],
    aurora: [
      [118, 162, 255],
      [94, 226, 199],
      [255, 206, 120],
    ],
  },
  day: {
    accent: [95, 208, 255],
    aurora: [
      [78, 128, 255],
      [71, 213, 255],
      [112, 255, 184],
    ],
  },
  evening: {
    accent: [255, 140, 92],
    aurora: [
      [126, 78, 221],
      [214, 94, 192],
      [255, 158, 96],
    ],
  },
  night: {
    accent: [128, 168, 245],
    aurora: [
      [98, 58, 205],
      [50, 175, 200],
      [50, 225, 115],
    ],
  },
};

export function getThemeNameForHour(hours) {
  if (hours < 6) {
    return 'night';
  }

  if (hours < 12) {
    return 'morning';
  }

  if (hours < 17) {
    return 'day';
  }

  if (hours < 20) {
    return 'evening';
  }

  return 'night';
}

export function getThemeForHour(hours) {
  return THEMES[getThemeNameForHour(hours)];
}
