export function buildLayout(bounds) {
  const centerX = Math.round(bounds.width / 2);

  if (bounds.isRound) {
    return {
      label: { x: centerX, y: 30 },
      meridiem: { x: centerX, y: 54 },
      time: { x: centerX, y: 110 },
      date: { x: centerX, y: 154 },
      statusLeft: { x: 44, y: 192 },
      statusCenter: { x: centerX, y: 192 },
      statusRight: { x: bounds.width - 44, y: 192 }
    };
  }

  return {
    label: { x: centerX, y: 42 },
    meridiem: { x: centerX, y: 70 },
    time: { x: centerX, y: 96 },
    date: { x: centerX, y: 128 },
    statusLeft: { x: 34, y: 176 },
    statusCenter: { x: centerX, y: 176 },
    statusRight: { x: bounds.width - 34, y: 176 }
  };
}
