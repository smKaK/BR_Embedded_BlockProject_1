


# Roadlight

Three-channel traffic light for the **ESP32-S3-DevKitC-1**, built with PlatformIO + Arduino framework. A compile-time CRTP state machine drives the red/yellow/green cycle; day/night mode can be driven by one of three sources at runtime — a phase timer, the BOOT button, or a photoresistor.

## Hardware

- **MCU:** ESP32-S3 @ 240 MHz (variant `N16R8` — 16 MB Flash, 8 MB PSRAM)
- **Board:** `esp32-s3-devkitc-1`
- **LEDs** (active-high through current-limiting resistors to GND):
  - Red → **GPIO 4**
  - Yellow → **GPIO 5**
  - Green → **GPIO 6**
- **Buttons** (active-low, `INPUT_PULLUP`, interrupt-sampled + debounced):
  - Pedestrian button → **GPIO 2**
  - On-board BOOT button → **GPIO 0**
- **Photoresistor (LDR) voltage divider on GPIO 1:**

  ```
    3V3 ──[ 10 kΩ ]──┬── GPIO 1 (ADC1_CH0)
                     │
                   [ LDR ]
                     │
                    GND
  ```

  With this wiring a darker room raises the ADC reading, which matches the default thresholds (`>= 2500` → night, `<= 1800` → day). Use an ADC1 pin only (GPIO 1–10 on S3); ADC2 conflicts with Wi-Fi.

## Runtime behaviour

After flashing, open the serial monitor at **115200 baud**.

### Day cycle

`DayRed` → `DayGreen` → `DayGreenBlink` → `DayYellow` → `DayYellowBlink` → `DayRed` → …

The two `*Blink` states are pre-change warnings — a fast-blinking version of the colour that's about to end. Durations (`kDay*Ms` in `include/traffic_light_config.h`) add up to roughly ten seconds per full cycle.

### Night cycle

A single `NightBlink` state that plays the `BlinkYellow` pattern (500 ms on / 500 ms off). The FSM stays parked here until a day/night source signals "go back to day".

### Pedestrian button (GPIO 2)

Press during `DayRed` → FSM jumps immediately to `DayGreen`. Ignored in any other state so the cycle can't be shortened mid-phase.

### Day/Night sources — only one is live at a time

`TrafficLightCoordinator` multiplexes three sources:

| Source        | Signal                                                             |
|---------------|--------------------------------------------------------------------|
| `timer`       | `PhaseTimer` auto-flips every 30 s (`kDayPhaseMs` / `kNightPhaseMs`). |
| `button`      | Short-press BOOT toggles day/night.                                |
| `lightSensor` | LDR reading crosses the hysteresis thresholds.                     |

**Long-press BOOT (≥ 800 ms) cycles** the active source in the order `timer → button → lightSensor → timer`. Serial prints `[coord] night source -> <name>` on every switch. When switching *to* `timer` or `lightSensor` the FSM force-syncs to whatever that source's current reading says it should be.

### Tunables

Everything user-facing lives as `constexpr` in `include/traffic_light_config.h`:

- Pins: `kPinRed/Yellow/Green`, `kPinPedButton`, `kPinBootButton`, `kPinLdr`.
- Day cycle durations: `kDayRedMs`, `kDayGreenMs`, `kDayGreenBlinkMs`, `kDayYellowMs`, `kDayYellowBlinkMs`.
- Night blink half-period: `kNightBlinkMs`.
- Phase timer: `kDayPhaseMs`, `kNightPhaseMs`.
- LDR: `kLdrDarkThreshold`, `kLdrBrightThreshold`, `kLdrSampleIntervalMs`.
- Button: `kDebounceMs`, `kBootHoldMs`.

## Architecture

```
main.cpp
  │ wires the globals (patterns, output, inputs, FSM, coordinator)
  │
  └── TrafficLightCoordinator
        │ runs the loop: flasher.update() + polls each source + fsm.tick()
        │
        ├── Flasher                  ─ advances the current IPattern's frame table,
        │     ├── MultiPinOutput    ← writes masks to GPIO 4/5/6
        │     └── IPattern          ← SolidRed / SolidYellow / SolidGreen /
        │                              BlinkGreen / BlinkYellow / BlinkYellowFast
        │
        ├── ModeManager              ─ pattern registry, exposes applyPattern(PatternId,…)
        │
        ├── TrafficLightFsm          ─ CRTP on fsm::StateMachine.
        │     states: DayRed, DayGreen, DayGreenBlink, DayYellow, DayYellowBlink, NightBlink
        │     events: fsm::Timeout, PedestrianPress, NightModeOn, NightModeOff
        │     on_enter calls modes.applyPattern(…) and arms set_timeout(…)
        │
        ├── PhaseTimer               ─ 30 s/30 s Day⇄Night flip emitter
        ├── LightSensor              ─ analogRead + hysteresis → Day/Night events
        ├── ButtonController (ped)   ─ GPIO 2 → Command::PedestrianPress
        └── ButtonController (boot)  ─ GPIO 0 → NightToggle (short) / CycleNightSource (long)
```

All wiring is done with designated-initialiser `Config` structs in `src/main.cpp`. For example:

```cpp
ButtonController gBootBtn(ButtonController::Config{
    .pin       = kPinBootButton,
    .debouncer = &gBootDeb,
    .sampler   = &gBootSampler,
    .activeLow = true,
    .pressCmd  = Command::NightToggle,
    .label     = "boot",
    .holdMs    = kBootHoldMs,
    .holdCmd   = Command::CycleNightSource,
});

TrafficLightCoordinator gCoord(TrafficLightCoordinator::Config{
    .flasher     = &gFlasher,
    .modes       = &gModes,
    .fsm         = &gFsm,
    .phase       = &gPhase,
    .pedButton   = &gPedBtn,
    .bootButton  = &gBootBtn,
    .lightSensor = &gLdr,
});
```

### The FSM (`include/fsm.hpp` + `TrafficLightFsm`)

`fsm::StateMachine<Derived, States...>` is a header-only CRTP template:

- States are plain value types held in a `std::variant`.
- The derived class provides `Trans handle(State&, const Event&)` overloads plus a catch-all `template <class S, class E> Trans handle(S&, const E&)`.
- `dispatch(ev, now_ms)` routes via `std::visit` — O(1), inlined to a jump table.
- `set_timeout(ms)` armed in `on_enter` synthesises a `fsm::Timeout` event when it expires, driving the day cycle.
- No heap, no RTTI, no exceptions.

`TrafficLightFsm`'s transitions:

- Day cycle is driven by `fsm::Timeout` (`DayRed → DayGreen → DayGreenBlink → DayYellow → DayYellowBlink → DayRed`).
- `PedestrianPress` only fires from `DayRed → DayGreen`.
- `NightModeOn` transitions any day state to `NightBlink`; `NightModeOff` returns `NightBlink → DayRed`.

### The coordinator (`TrafficLightCoordinator`)

One `begin(now_ms)` + one `tick(now_ms)`. `tick()` delegates to small per-source helpers:

```cpp
void TrafficLightCoordinator::tick(uint32_t now_ms) {
    flasher_->update();
    pollPedestrian(now_ms);
    pollBootButton(now_ms);
    pollPhase(now_ms);
    pollLightSensor(now_ms);
    fsm_->tick(now_ms);
}
```

Each `poll*` helper reads its source and only dispatches to the FSM when that source is the active one. Adding a new source is an extra `poll*()` method + a new enum value.

### Debouncing

Mechanical buttons rattle when pressed; a debouncer watches the raw signal and commits a stable level. Three algorithms are available (`IDebounceAlgo` impls):

- **HysteresisDebounce** — commit once the raw level has held steady for `stableMs`. Predictable wall-clock latency; the default.
- **IntegratorDebounce** — each sample votes; counter flips only at saturation. Graceful with isolated noise, tied to sample rate.
- **ShiftRegisterDebounce** — require `width` consecutive clean samples to agree. Strictest; biggest latency.

Sampling is factored behind `IButtonSampler`:

- **PollingSampler** — sample every tick.
- **InterruptSampler** — only sample when a `CHANGE` ISR fired or while the debouncer is still settling. Used by both buttons.

### Long-press detection

`ButtonController::Config` accepts optional `holdMs` + `holdCmd`. When `holdMs > 0` the controller switches to hold-aware mode:

- `pressCmd` fires **on release**, only if released before `holdMs` elapsed.
- `holdCmd` fires **once while still holding** at the `holdMs` mark.

`holdMs == 0` (the default) keeps the legacy rising-edge behaviour used by the pedestrian button.

## Build & flash

```bash
pio run                          # build
pio run --target upload          # flash
pio device monitor               # serial monitor (115200)
pio run --target clean           # clean
```

Build flags live in `platformio.ini`. C++17 (`-std=gnu++17`), no extra libraries.

## Adding a new pattern

1. Create `include/patterns/MyPattern.h` deriving from `pflash::IPattern`.
2. Create `src/patterns/MyPattern.cpp` with a `constexpr Frame kFrames[]` table — each frame is `{ duration_ms, channel_mask }`. Bit `N` maps to channel `N` in the `MultiPinOutput` pin list (bit 0 = red, bit 1 = yellow, bit 2 = green).
3. Add a `PatternId` enum value matching the index at which the pattern is registered in `ModeManager`'s Config.
4. Instantiate it and include it in the `patterns` list in `src/main.cpp`.
5. Reference it from a `TrafficLightFsm::on_enter(State&)` via `modes_->applyPattern(PatternId::X, "fsm")`.

## Adding a new day/night source

1. Implement a polled class with an `Event poll(uint32_t now_ms)` method — or implement `IController` if button-like semantics fit.
2. Add a field to `TrafficLightCoordinator::Config` and a new `NightSource` enum value.
3. Add a `pollX()` helper in the coordinator that dispatches events only when `activeSource_` matches, and call it from `tick()`.
4. Extend `cycleSource()` and `forceSyncToSource()` so the new source joins the rotation and the FSM re-syncs when switched to.

## Project layout

```
include/              project headers
├── Flasher.h, FlasherBuilder.h
├── ModeManager.h, PatternId.h, IPattern.h, IOutputStrategy.h
├── PhaseTimer.h, LightSensor.h
├── TrafficLightFsm.h, TrafficLightCoordinator.h
├── fsm.hpp
├── traffic_light_config.h
├── outputs/          MultiPinOutput
├── patterns/         SolidRed/Yellow/Green, BlinkYellow, BlinkGreen, BlinkYellowFast
├── inputs/           IController, ButtonController, InterruptSampler, PollingSampler
└── debounce/         Debouncer + IDebounceAlgo implementations
src/                  matching .cpp files + main.cpp
platformio.ini        build environment
```
