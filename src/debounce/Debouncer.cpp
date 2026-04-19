#include "debounce/Debouncer.h"

namespace pflash {

Debouncer::Debouncer(IDebounceAlgo* algo) : algo_(algo) {}

void Debouncer::begin(bool initialLevel, uint32_t /*now_ms*/) {
    stable_ = initialLevel;
    if (algo_) {
        algo_->reset(initialLevel);
    }
}

Debouncer::Edge Debouncer::update(bool rawLevel, uint32_t now_ms) {
    if (!algo_) {
        return Edge::None;
    }
    const bool next = algo_->update(rawLevel, now_ms);
    if (next == stable_) {
        return Edge::None;
    }
    stable_ = next;
    return next ? Edge::Rising : Edge::Falling;
}

} // namespace pflash
