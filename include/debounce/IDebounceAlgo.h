#pragma once

#include <cstdint>

namespace pflash {

class IDebounceAlgo {
public:
    virtual ~IDebounceAlgo() = default;

    virtual void        reset(bool initialLevel)                   = 0;
    virtual bool        update(bool rawLevel, uint32_t now_ms)     = 0;

    // True when the algorithm has internal state that may commit purely from
    // the passage of time (no new raw samples required). The driver should
    // keep calling update() until this returns false.
    virtual bool        pending() const                            = 0;

    virtual const char* name() const                               = 0;
};

} // namespace pflash
