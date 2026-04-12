export function buildLayout({ width, height, isRound }) {
  const cx = Math.round(width / 2);

  if (isRound && width >= 250) {
    // large-round: gabbro 260×260
    return {
      label:        { x: cx,           y:  30 },
      meridiem:     { x: cx,           y:  54 },
      time:         { x: cx,           y: 110 },
      date:         { x: cx,           y: 154 },
      statusLeft:   { x: 44,           y: 192 },
      statusCenter: { x: cx,           y: 192 },
      statusRight:  { x: width - 44,   y: 192 },
    };
  }

  if (isRound) {
    // small-round: chalk 180×180
    return {
      label:        { x: cx,           y:  22 },
      meridiem:     { x: cx,           y:  40 },
      time:         { x: cx,           y:  80 },
      date:         { x: cx,           y: 112 },
      statusLeft:   { x: 30,           y: 140 },
      statusCenter: { x: cx,           y: 140 },
      statusRight:  { x: width - 30,   y: 140 },
    };
  }

  if (width >= 190) {
    // large-rect: emery 200×228
    return {
      label:        { x: cx,           y:  42 },
      meridiem:     { x: cx,           y:  70 },
      time:         { x: cx,           y:  96 },
      date:         { x: cx,           y: 128 },
      statusLeft:   { x: 34,           y: 176 },
      statusCenter: { x: cx,           y: 176 },
      statusRight:  { x: width - 34,   y: 176 },
    };
  }

  // small-rect: basalt/diorite 144×168
  return {
    label:        { x: cx,           y:  30 },
    meridiem:     { x: cx,           y:  54 },
    time:         { x: cx,           y:  76 },
    date:         { x: cx,           y: 106 },
    statusLeft:   { x: 24,           y: 144 },
    statusCenter: { x: cx,           y: 144 },
    statusRight:  { x: width - 24,   y: 144 },
  };
}
