# Flagship

Flagship is a premium utility-first Pebble watchface designed for the Spring 2026 Pebble App Contest.

## Current Scope

- Large time display
- Compact uppercase date
- Battery status
- Connection status
- Layout targets for `emery` and `gabbro`

## Development Workflow

### 1. Work locally

Use this repository as the source of truth.

```bash
cd watchfaces/flagship
npm test
```

### 2. Push to GitHub

After creating your remote repository:

```bash
git remote add origin <your-github-repo-url>
git push -u origin main
```

### 3. Bring the project into the Pebble cloud workflow

Use the rePebble developer environment to open or import the project from GitHub once the repository is pushed.

## Testing Plan

### Local now

- `npm test` verifies pure formatting and layout logic

### After Pebble CLI is installed

```bash
pebble build
pebble install --emulator emery
pebble install --emulator gabbro
pebble logs
```

### On device

- Install via the Pebble phone app developer connection flow
- Confirm legibility outdoors and on wrist
- Confirm the minute tick refresh is stable

## Submission Materials

Prepare these before appstore submission:

- final `.pbw`
- GitHub repository URL
- support email
- description copy
- screenshots for `emery`
- screenshots for `gabbro`
