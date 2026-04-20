#pragma once

#include <cstdint>

namespace pflash {

// Pins
constexpr uint8_t kPinRed         = 4;
constexpr uint8_t kPinYellow      = 5;
constexpr uint8_t kPinGreen       = 6;
constexpr uint8_t kPinPedButton   = 2;
constexpr uint8_t kPinBootButton  = 0;

// Debounce + serial
constexpr uint32_t kDebounceMs    = 20;
constexpr uint32_t kSerialBaud    = 115200;

// Day cycle durations (total unchanged at 10 s: 5 + 3 + 1 + 0.5 + 0.5)
constexpr uint32_t kDayRedMs         = 5000;
constexpr uint32_t kDayGreenMs       = 3000;
constexpr uint32_t kDayGreenBlinkMs  = 2000;
constexpr uint32_t kDayYellowMs      = 3000;
constexpr uint32_t kDayYellowBlinkMs = 2000;

// Night blink half-period
constexpr uint32_t kNightBlinkMs  = 500;

// Day/Night phase timer
constexpr uint32_t kDayPhaseMs    = 30000;
constexpr uint32_t kNightPhaseMs  = 30000;

} // namespace pflash
