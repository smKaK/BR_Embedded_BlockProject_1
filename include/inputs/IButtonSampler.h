#pragma once

#include <cstdint>

namespace pflash {

// Decides, each tick, whether ButtonController should read the pin and feed
// its Debouncer. Lets us pick polling vs. interrupt-gated sampling per button
// without branching inside ButtonController.
class IButtonSampler {
public:
    virtual ~IButtonSampler() = default;

    // Called once from ButtonController::begin() AFTER pinMode is configured,
    // so implementations can attach an ISR or record per-pin state.
    virtual void        begin(uint8_t pin, bool activeLow) = 0;
    virtual bool        shouldSample(uint32_t now_ms)      = 0;
    virtual const char* name() const                       = 0;
};

} // namespace pflash
