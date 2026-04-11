# Flagship Pebble Watchface

A contest-ready Pebble watchface project focused on polished daily utility for Time 2 and Round 2.

## Goals

- Ship a professional first public watchface for the Spring 2026 Pebble App Contest
- Support `emery` and `gabbro` from the start
- Keep core display logic testable locally before integrating with the Pebble toolchain

## Repository Layout

- `docs/` - contest notes, design choices, submission checklist
- `watchfaces/flagship/` - the Pebble project

## Local Development

This repository is structured so the pure formatting and layout logic can be tested with Node before the Pebble SDK is installed locally.

```bash
cd watchfaces/flagship
npm test
```

Later, when the Pebble CLI is installed:

```bash
pebble build
pebble install --emulator emery
```
