# Flagship

Flagship is a hobby Pebble watchface focused on calm, useful daily information.

## Current Scope

- large time display
- compact uppercase date
- battery status
- connection status
- layout support for `emery` and `gabbro`

## Development Workflow

### Work locally first

```bash
cd watchfaces/flagship
npm test
```

### Build with the Pebble CLI

```bash
cd watchfaces/flagship
pebble build
```

### Run on emulators

```bash
cd watchfaces/flagship
pebble install --emulator emery
pebble install --emulator gabbro
```

### Useful extras

```bash
pebble logs --emulator emery
pebble logs --emulator gabbro
pebble screenshot --emulator emery --no-open emery.png
pebble screenshot --emulator gabbro --no-open gabbro.png
```

## What Gets Tested Locally

The Node tests cover the pure logic parts of the watchface:

- time formatting
- date formatting
- battery and connection labels
- layout behavior for round and rectangular targets
- combined face model generation

## Structure

- `src/common/` - shared watchface logic
- `src/c/` - native Pebble app entry point
- `src/pkjs/` - PebbleKit JS code
- `src/embeddedjs/` - Moddable embedded JavaScript
- `tests/` - local logic tests
- `screenshots/` - reference images
