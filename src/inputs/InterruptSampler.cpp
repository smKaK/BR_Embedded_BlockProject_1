#include "inputs/InterruptSampler.h"

#include <Arduino.h>

namespace pflash {

void IRAM_ATTR InterruptSampler::onIsr(void* arg) {
    auto* self = static_cast<InterruptSampler*>(arg);
    self->flag_ = true;
}

void InterruptSampler::begin(uint8_t pin, bool /*activeLow*/) {
    pin_  = pin;
    flag_ = true;
    attachInterruptArg(digitalPinToInterrupt(pin_),
                       &InterruptSampler::onIsr, this, CHANGE);
}

bool InterruptSampler::shouldSample(uint32_t /*now_ms*/) {
    if (flag_) {
        flag_ = false;
        return true;
    }
    return false;
}

} // namespace pflash
