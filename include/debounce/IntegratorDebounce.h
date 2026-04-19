#pragma once

#include "IDebounceAlgo.h"

namespace pflash {

// Saturating up/down counter: +1 per high sample, -1 per low, clamped to [0, max].
// Output flips to high when counter saturates at max, back to low when it drops to 0.
class IntegratorDebounce final : public IDebounceAlgo {
public:
    explicit IntegratorDebounce(uint8_t maxCount = 5);

    void        reset(bool initialLevel)                   override;
    bool        update(bool rawLevel, uint32_t now_ms)     override;
    bool        pending() const                            override { return false; }
    const char* name() const                               override;

private:
    uint8_t maxCount_;
    uint8_t count_{0};
    bool    stable_{false};
};

} // namespace pflash
