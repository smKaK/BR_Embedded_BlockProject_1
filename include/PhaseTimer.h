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

    explicit PhaseTimer(const Config& c)
        : dayMs_(c.dayMs), nightMs_(c.nightMs) {}

    void begin(uint32_t now_ms) {
        phaseStart_ms_ = now_ms;
        isDay_         = true;
    }

    Flip tick(uint32_t now_ms) {
        const uint32_t dur = isDay_ ? dayMs_ : nightMs_;
        if ((uint32_t)(now_ms - phaseStart_ms_) < dur) {
            return Flip::None;
        }
        isDay_         = !isDay_;
        phaseStart_ms_ = now_ms;
        return isDay_ ? Flip::ToDay : Flip::ToNight;
    }

    bool isDay() const { return isDay_; }

private:
    uint32_t dayMs_;
    uint32_t nightMs_;
    uint32_t phaseStart_ms_ = 0;
    bool     isDay_         = true;
};

} // namespace pflash
