#include "debounce/IntegratorDebounce.h"

namespace pflash {

IntegratorDebounce::IntegratorDebounce(uint8_t maxCount)
    : maxCount_(maxCount == 0 ? 1 : maxCount) {}

void IntegratorDebounce::reset(bool initialLevel) {
    stable_ = initialLevel;
    count_  = initialLevel ? maxCount_ : 0;
}

bool IntegratorDebounce::update(bool rawLevel, uint32_t /*now_ms*/) {
    if (rawLevel) {
        if (count_ < maxCount_) { ++count_; }
    } else {
        if (count_ > 0)         { --count_; }
    }
    if (count_ == maxCount_)    { stable_ = true;  }
    else if (count_ == 0)       { stable_ = false; }
    return stable_;
}

const char* IntegratorDebounce::name() const {
    return "Integrator";
}

} // namespace pflash
