export function buildLayout(bounds) {
  const centerX = Math.round(bounds.width / 2);

  if (bounds.isRound) {
    return {
      time: { x: centerX, y: 78 },
      date: { x: centerX, y: 106 },
      statusLeft: { x: 40, y: 128 },
      statusRight: { x: bounds.width - 40, y: 128 }
    };
  }

  return {
    time: { x: centerX, y: 96 },
    date: { x: centerX, y: 128 },
    statusLeft: { x: 34, y: 176 },
    statusRight: { x: bounds.width - 34, y: 176 }
  };
}
