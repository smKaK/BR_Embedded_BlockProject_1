#pragma once

#include "IButtonSampler.h"

namespace pflash {

// Always samples. Preserves the classic poll-every-tick behavior.
class PollingSampler final : public IButtonSampler {
public:
    void        begin(uint8_t /*pin*/, bool /*activeLow*/) override {}
    bool        shouldSample(uint32_t /*now_ms*/)          override { return true; }
    const char* name() const                               override { return "polling"; }
};

} // namespace pflash
