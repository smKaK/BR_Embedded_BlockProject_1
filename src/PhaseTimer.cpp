#include "PhaseTimer.h"

namespace pflash {

PhaseTimer::PhaseTimer(const Config& c)
    : dayMs_(c.dayMs), nightMs_(c.nightMs) {}

void PhaseTimer::begin(uint32_t now_ms) {
    phaseStart_ms_ = now_ms;
    isDay_         = true;
}

PhaseTimer::Flip PhaseTimer::tick(uint32_t now_ms) {
    const uint32_t dur = isDay_ ? dayMs_ : nightMs_;
    if ((uint32_t)(now_ms - phaseStart_ms_) < dur) {
        return Flip::None;
    }
    isDay_         = !isDay_;
    phaseStart_ms_ = now_ms;
    return isDay_ ? Flip::ToDay : Flip::ToNight;
}

} // namespace pflash
