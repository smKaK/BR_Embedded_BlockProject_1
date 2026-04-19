# PoliceFlash

Two-channel police-style LED flasher for the **ESP32-S3-DevKitC-1**, built with PlatformIO + Arduino framework. Runs several blink patterns and lets you switch between them live — over the serial console or via hardware buttons — with a pluggable input + debounce stack behind the scenes.

## Hardware

- **MCU:** ESP32-S3 @ 240 MHz (variant `N16R8` — 16 MB Flash, 8 MB PSRAM)
- **Board:** `esp32-s3-devkitc-1`
- **LEDs:**
  - Red → **GPIO 4**
  - Blue → **GPIO 5**
- **Buttons (active-low, `INPUT_PULLUP`):**
  - External button → **GPIO 2** (interrupt-gated sampling)
  - On-board BOOT button → **GPIO 0** (polled sampling)

Wire each LED through a current-limiting resistor to ground. Wire each external button between its GPIO and GND.

## Runtime Control

After flashing, open the serial monitor at **115200 baud**. Either type a command and press Enter, or press one of the buttons:

| Input            | Effect                               |
|------------------|--------------------------------------|
| `p0`             | Alternating — red/blue ping-pong     |
| `p1`             | DoubleBlinkPolice *(default)*        |
| `p2`             | SOS — Morse `... --- ...` on both    |
| `?`              | Print current pattern name           |
| any other text   | Print help                           |
| External / BOOT  | Cycle to the next pattern            |

## Architecture

The firmware splits into three cooperating layers. `Flasher` still drives the LEDs from pattern frames; a new `ModeManager` owns the pattern registry and the list of input controllers; each controller turns its own source (serial / button) into a uniform `ControlEvent` stream.

```
main.cpp
  │
  ├── Flasher  ──  ticks through the active IPattern and writes masks to the output
  │     ├── IOutputStrategy  ←  MultiPinOutput (digitalWrite on N pins)
  │     └── IPattern         ←  AlternatingPattern
  │                             DoubleBlinkPolicePattern
  │                             SosPattern
  │
  └── ModeManager  ──  polls IControllers every tick, dispatches ControlEvents to Flasher
        ├── SerialController           ←  parses "p0/p1/p2/?" from UART
        └── ButtonController (x N)     ←  pin → Debouncer → edge → Command::NextPattern
              ├── IDebounceAlgo        ←  Hysteresis / Integrator / ShiftRegister
              └── IButtonSampler       ←  PollingSampler / InterruptSampler
```

All wiring is done with designated-initializer `Config` structs, e.g.:

```cpp
pflash::ButtonController gExtBtn({
    .pin       = kPinExtButton,
    .debouncer = &gExtDeb,
    .sampler   = &gExtSampler,
    .label     = "ext",
});

pflash::ModeManager gModes({
    .flasher      = &gFlasher,
    .patterns     = { &gAlternating, &gDoubleBlink, &gSos },
    .controllers  = { &gSerialCtl, &gExtBtn, &gBootBtn },
    .defaultIndex = pflash::idx(pflash::PatternId::DoubleBlinkPolice),
});
```

### Debouncing

`Debouncer` is the driver; the actual algorithm lives behind `IDebounceAlgo` so it can be swapped per button:

- **HysteresisDebounce** — commits a new level only after it has held steady for `Nms`.
- **IntegratorDebounce** — saturating up/down counter; resists noisy spikes.
- **ShiftRegisterDebounce** — classic N-sample shift register; commits on all-ones / all-zeros.

### Sampling strategies

`IButtonSampler::shouldSample(now_ms)` decides each tick whether `ButtonController` should read the pin:

- **PollingSampler** — always samples (simple, slightly more CPU).
- **InterruptSampler** — only samples when a GPIO ISR flagged a change, or while the debouncer is still settling.

### Adding a new pattern

1. Create `include/patterns/MyPattern.h` deriving from `pflash::IPattern`.
2. Create `src/patterns/MyPattern.cpp` with a `constexpr Frame kFrames[]` table — each frame is `{ duration_ms, channel_mask }`.
3. Add it to `PatternId` (keeps serial `pN` indices in sync) and include it in the `ModeManager` `patterns` list in `main.cpp`.

A `Frame`'s `channel_mask` bit `N` maps to the LED at index `N` in the `MultiPinOutput` pin list.

### Adding a new input source

1. Implement `pflash::IController` (`begin()`, `poll()`, `name()`) and emit `ControlEvent`s.
2. Instantiate it and add it to the `ModeManager` `controllers` list.

## Project Layout

```
include/          project headers
├── Flasher.h, FlasherBuilder.h
├── ModeManager.h, PatternId.h
├── IOutputStrategy.h, IPattern.h
├── outputs/      concrete output strategies
├── patterns/     concrete pattern headers
├── inputs/       IController, SerialController, ButtonController, samplers
└── debounce/     Debouncer + IDebounceAlgo implementations
src/              matching .cpp files + main.cpp
lib/              local libraries
test/             unit tests (PlatformIO)
platformio.ini    build environment
```
