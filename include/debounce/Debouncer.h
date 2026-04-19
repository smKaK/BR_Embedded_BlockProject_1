#pragma once

#include "IDebounceAlgo.h"

namespace pflash {

class Debouncer {
public:
    enum class Edge : uint8_t { None, Rising, Falling };

    explicit Debouncer(IDebounceAlgo* algo);

    void  begin(bool initialLevel, uint32_t now_ms);
    Edge  update(bool rawLevel, uint32_t now_ms);
    bool  stableLevel() const { return stable_; }

    // Delegated from the algorithm: true while the debouncer still wants
    // periodic updates even without any raw-level change.
    bool  pending() const { return algo_ && algo_->pending(); }

    const IDebounceAlgo* algo() const { return algo_; }

private:
    IDebounceAlgo* algo_;
    bool           stable_{false};
};

} // namespace pflash
