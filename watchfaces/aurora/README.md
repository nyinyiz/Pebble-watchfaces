# Aurora

> Premium utility-first Pebble watchface — aurora borealis visual design.
> Targets **emery** (Pebble Time 2, 200×228) and **gabbro** (Pebble Round 2, 260×260).

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
# Test pure logic (no SDK needed)
npm test

# Full build
pebble build

# Install to emulator
pebble install --emulator emery
pebble install --emulator gabbro

# Useful extras
pebble screenshot --no-open --emulator emery emery.png
pebble screenshot --no-open --emulator gabbro gabbro.png
pebble logs --emulator emery
```

## Source Layout

```
src/
├── common/          Pure JS — format, layout, model, animation (unit-tested)
├── embeddedjs/      On-device rendering (Moddable XS / Poco)
├── c/               Pebble app entry point (minimal C wrapper)
└── pkjs/            Companion phone app
tests/               Node.js native test runner (12 tests)
```

## Testing

The test suite covers all pure logic without requiring a Pebble SDK:

- `format.test.mjs` — time, date, battery, Bluetooth label formatting
- `layout.test.mjs` — position maps for round and rectangular screens
- `model.test.mjs` — watch state → display model aggregation
- `animation.test.mjs` — second-progress, band phases, sweep position
