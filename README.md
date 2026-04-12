# Pebble Watchfaces Collection

A collection of premium, high-performance native Pebble watchfaces built with the Pebble C SDK.

---

## 1. Matrix Digital Rain

> Experience the iconic movie's visual aesthetic on your wrist.

<br>

<img src="watchfaces/matrix/screenshots/banner_basalt.png" alt="Matrix banner" width="100%">

<br>

### Live on Device

| Pebble Time 2 (Emery) | Pebble Round 2 (Gabbro) |
| :---: | :---: |
| <img src="watchfaces/matrix/screenshots/screenshot_emery_live.png" alt="Matrix on emery" width="180"> | <img src="watchfaces/matrix/screenshots/screenshot_gabbro_live.png" alt="Matrix on gabbro" width="220"> |
| 200 × 228 px · rectangular | 260 × 260 px · round |

### Features
- **Dynamic Digital Rain** — 10 FPS cascading character animation.
- **Centered Bold Time** — HH:MM:SS in LECO 32 Bold.
- **Real-time Stats** — Battery percentage (+/- charging state) and daily step count.
- **Full Date** — Day of week and date display.
- **24h Support** — Auto-detects system time format.

---

## 2. Dual Time

> Minimalist dual-timezone watchface for world travelers.

<br>

<img src="watchfaces/dual/screenshots/banner-emery.png" alt="Dual Time banner" width="100%">

<br>

### Live on Device

| Pebble Time 2 (Emery) | Pebble Round 2 (Gabbro) |
| :---: | :---: |
| <img src="watchfaces/dual/screenshots/emery-digital-final.png" alt="Dual on emery" width="180"> | <img src="watchfaces/dual/screenshots/gabbro-digital-final.png" alt="Dual on gabbro" width="220"> |
| 200 × 228 px · rectangular | 260 × 260 px · round |

### Features
- **Dual Timezones** — Track local and secondary time zones simultaneously.
- **Switchable Modes** — Supports both digital and analog rendering.
- **Clean Aesthetic** — Minimalist design optimized for maximum readability.
- **Multi-platform** — Full support for round, rectangular, and B&W Pebble displays.

---

## 3. Aurora

> A premium utility-first watchface with an aurora borealis visual design.

<br>

<img src="watchfaces/aurora/screenshots/banner_emery.png" alt="Aurora banner — emery" width="100%">

<br>

### Live on Device

| Pebble Time 2 (Emery) | Pebble Round 2 (Gabbro) |
| :---: | :---: |
| <img src="watchfaces/aurora/screenshots/screenshot_emery.png" alt="Aurora on emery" width="180"> | <img src="watchfaces/aurora/screenshots/screenshot_gabbro.png" alt="Aurora on gabbro" width="220"> |
| 200 × 228 px · rectangular | 260 × 260 px · round |

### Features
- **Animated Aurora** — Three shifting sine-wave bands (violet → teal → green).
- **Star Field** — Twinkling stars with subtle per-second updates.
- **Time-of-day Colors** — Dawn, Day, and Night accent themes.
- **Detailed Status** — Battery bar, seconds pulse, and Bluetooth state.

---

## Quick Start

### Build & Install Matrix
```bash
cd watchfaces/matrix
~/.local/bin/pebble build
~/.local/bin/pebble install --emulator emery
```

### Build & Install Dual Time
```bash
cd watchfaces/dual
~/.local/bin/pebble build
~/.local/bin/pebble install --emulator emery
```

### Build & Install Aurora
```bash
cd watchfaces/aurora
~/.local/bin/pebble build
~/.local/bin/pebble install --emulator emery
```

## Project Structure
```
.
├── watchfaces/
│   ├── aurora/          # Aurora watchface (Native C)
│   ├── dual/            # Dual Time watchface (Native C)
│   └── matrix/          # Matrix Digital Rain (Native C)
└── README.md            # You are here
```