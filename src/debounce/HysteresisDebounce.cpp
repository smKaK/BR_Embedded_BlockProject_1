#include "debounce/HysteresisDebounce.h"

namespace pflash {

HysteresisDebounce::HysteresisDebounce(uint32_t stableMs) : stableMs_(stableMs) {}

void HysteresisDebounce::reset(bool initialLevel) {
    stable_         = initialLevel;
    candidate_      = initialLevel;
    candidateSince_ = 0;
}

bool HysteresisDebounce::update(bool rawLevel, uint32_t now_ms) {
    if (rawLevel != candidate_) {
        candidate_      = rawLevel;
        candidateSince_ = now_ms;
        return stable_;
    }
    if (candidate_ != stable_ && (now_ms - candidateSince_) >= stableMs_) {
        stable_ = candidate_;
    }
    return stable_;
}

bool HysteresisDebounce::pending() const {
    return candidate_ != stable_;
}

const char* HysteresisDebounce::name() const {
    return "Hysteresis";
}

} // namespace pflash
