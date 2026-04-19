#pragma once

#include "Flasher.h"

namespace pflash {

class FlasherBuilder {
public:
    FlasherBuilder& withOutput(IOutputStrategy* out);
    FlasherBuilder& withPattern(IPattern* p);

    Flasher build();

private:
    IOutputStrategy* out_{nullptr};
    IPattern*        pat_{nullptr};
};

} // namespace pflash
