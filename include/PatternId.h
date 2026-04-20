#pragma once

#include <cstdint>

namespace pflash {

// Pattern identity. The underlying values MUST match the order patterns are
// registered with ModeManager's Config::patterns list (see main.cpp).
enum class PatternId : uint8_t {
    SolidRed        = 0,
    SolidYellow     = 1,
    SolidGreen      = 2,
    BlinkYellow     = 3,
    BlinkGreen      = 4,
    BlinkYellowFast = 5,
};

constexpr uint8_t idx(PatternId p) noexcept {
    return static_cast<uint8_t>(p);
}

} // namespace pflash
