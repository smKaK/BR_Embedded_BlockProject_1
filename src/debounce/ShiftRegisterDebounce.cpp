#include "debounce/ShiftRegisterDebounce.h"

namespace pflash {

ShiftRegisterDebounce::ShiftRegisterDebounce(uint8_t width, uint32_t sampleIntervalMs)
    : width_(width == 0 ? 1 : (width > 32 ? 32 : width)),
      sampleIntervalMs_(sampleIntervalMs),
      mask_(width_ >= 32 ? 0xFFFFFFFFu : ((1u << width_) - 1u)) {}

void ShiftRegisterDebounce::reset(bool initialLevel) {
    stable_     = initialLevel;
    reg_        = initialLevel ? mask_ : 0u;
    lastSample_ = 0;
    primed_     = false;
}

bool ShiftRegisterDebounce::update(bool rawLevel, uint32_t now_ms) {
    if (primed_ && (now_ms - lastSample_) < sampleIntervalMs_) {
        return stable_;
    }
    lastSample_ = now_ms;
    primed_     = true;

    reg_ = ((reg_ << 1) | (rawLevel ? 1u : 0u)) & mask_;

    if      (reg_ == mask_) { stable_ = true;  }
    else if (reg_ == 0u)    { stable_ = false; }
    return stable_;
}

const char* ShiftRegisterDebounce::name() const {
    return "ShiftRegister";
}

} // namespace pflash
