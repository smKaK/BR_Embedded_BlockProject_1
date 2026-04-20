#pragma once

#include <cstdint>

namespace pflash {

class PhaseTimer {
public:
    enum class Flip : uint8_t { None, ToDay, ToNight };

    struct Config {
        uint32_t dayMs;
        uint32_t nightMs;
    };

    explicit PhaseTimer(const Config& c);

    void begin(uint32_t now_ms);
    Flip tick(uint32_t now_ms);

    bool isDay() const { return isDay_; }

private:
    uint32_t dayMs_;
    uint32_t nightMs_;
    uint32_t phaseStart_ms_ = 0;
    bool     isDay_         = true;
};

} // namespace pflash
