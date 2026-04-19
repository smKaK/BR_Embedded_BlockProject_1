#pragma once

#include <cstdint>

namespace pflash {

// Pattern identity. The underlying values MUST match the order patterns are
// registered with ModeManager's Config::patterns list (see main.cpp).
enum class PatternId : uint8_t {
    Alternating       = 0,
    DoubleBlinkPolice = 1,
    SOS               = 2,
};

constexpr uint8_t idx(PatternId p) noexcept {
    return static_cast<uint8_t>(p);
}

} // namespace pflash
