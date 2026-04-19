#pragma once

#include "IDebounceAlgo.h"

namespace pflash {

// Accepts a new level only after it stays unchanged for `stableMs`.
class HysteresisDebounce final : public IDebounceAlgo {
public:
    explicit HysteresisDebounce(uint32_t stableMs = 20);

    void        reset(bool initialLevel)                   override;
    bool        update(bool rawLevel, uint32_t now_ms)     override;
    bool        pending() const                            override;
    const char* name() const                               override;

private:
    uint32_t stableMs_;
    bool     stable_{false};
    bool     candidate_{false};
    uint32_t candidateSince_{0};
};

} // namespace pflash
