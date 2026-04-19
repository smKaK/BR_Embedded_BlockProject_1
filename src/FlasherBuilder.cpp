#include "FlasherBuilder.h"

namespace pflash {

FlasherBuilder& FlasherBuilder::withOutput(IOutputStrategy* out) {
    out_ = out;
    return *this;
}

FlasherBuilder& FlasherBuilder::withPattern(IPattern* p) {
    pat_ = p;
    return *this;
}

Flasher FlasherBuilder::build() {
    return Flasher(out_, pat_);
}

} // namespace pflash
