# Aurora

> Premium utility-first Pebble watchface — aurora borealis visual design.
> Native Pebble C watchface for `basalt`, `chalk`, `diorite`, `emery`, `flint`, and `gabbro`.

## Features

| Feature | Detail |
| --- | --- |
| Aurora animation | Three sine-wave bands (violet → teal → green) driven by real-time seconds |
| Star field | Twinkling points above the mountain horizon |
| Mountain silhouette | Parabolic peaks with snow caps *(rectangular only)* |
| Second-sweep pulse | Accent line traverses the divider once per minute |
| Time-of-day accents | Amber · sky blue · violet, shifting at 06:00 / 12:00 / 18:00 |
| Status row | Battery % · live seconds · Bluetooth state |
| 12 h / 24 h | Auto-detected from watch settings |

## Development

```bash
# Test pure logic (no Pebble SDK needed)
npm run test:native

# Full build
/Users/nyinyizaw/.local/bin/pebble build

# Install to emulator
/Users/nyinyizaw/.local/bin/pebble install --emulator emery build/aurora.pbw
/Users/nyinyizaw/.local/bin/pebble install --emulator gabbro build/aurora.pbw

# Useful extras
/Users/nyinyizaw/.local/bin/pebble screenshot --no-open --emulator emery emery.png
/Users/nyinyizaw/.local/bin/pebble screenshot --no-open --emulator gabbro gabbro.png
/Users/nyinyizaw/.local/bin/pebble logs --emulator emery
```

## Source Layout

```
src/
└── c/
    ├── main.c            Pebble watchface runtime, rendering, event subscriptions
    ├── aurora_logic.c    Pure C logic for layout profile, theme, labels, animation
    └── aurora_logic.h
tests/
└── aurora_logic_test.c   Host-side native C test harness
```

## Testing

The native test harness covers the cross-platform logic without requiring a Pebble SDK:

- time, date, battery, and Bluetooth label formatting
- profile mapping for 144×168 / 180×180 / 200×228 / 260×260 layouts
- second sweep and animation phase derivation
