#pragma once

#include "IButtonSampler.h"

namespace pflash {

// Attaches a CHANGE interrupt to the pin. shouldSample() returns true when
// the ISR has fired since the last call (and clears the flag). It knows
// nothing about debouncers — the caller is responsible for any additional
// periodic re-sampling its debouncer may need.
class InterruptSampler final : public IButtonSampler {
public:
    InterruptSampler() = default;

    void        begin(uint8_t pin, bool activeLow) override;
    bool        shouldSample(uint32_t now_ms)      override;
    const char* name() const                       override { return "isr"; }

private:
    static void onIsr(void* arg);   // IRAM_ATTR applied in the .cpp

    volatile bool flag_{true};      // force the first sample
    uint8_t       pin_{0};
};

} // namespace pflash
