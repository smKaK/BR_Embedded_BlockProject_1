# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Flash Commands

```bash
# Build the project
pio run

# Upload to connected ESP32-S3
pio run --target upload

# Build and upload
pio run --target upload --environment esp32-s3-devkitc-1

# Open serial monitor
pio device monitor

# Clean build artifacts
pio run --target clean
```

## Hardware Target

**Board:** ESP32-S3-DevKitC-1 (`esp32-s3-devkitc-1`)
- MCU: ESP32-S3 @ 240 MHz
- Flash: 16MB (QIO mode), variant `N16R8`
- PSRAM: 8MB (OPI mode), enabled via `-DBOARD_HAS_PSRAM`
- Partition table: `default_16MB.csv`
- Framework: Arduino

## Project Structure

- `src/main.cpp` — Arduino entry point (`setup()` + `loop()`)
- `include/` — Project header files
- `lib/` — Project-local libraries (private dependencies)
- `platformio.ini` — Build environment configuration

## Key Configuration Notes

The memory type is set to `qio_opi` (QIO flash + OPI PSRAM), which is required for the N16R8 chip variant. If changing flash/PSRAM settings, all four related flags must stay consistent:
```ini
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_upload.flash_size = 16MB
```

PSRAM is accessible at runtime because `BOARD_HAS_PSRAM` is defined. Use `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` to explicitly allocate in PSRAM.

## Git Commits

Do not add `Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>` footers to commit messages.
