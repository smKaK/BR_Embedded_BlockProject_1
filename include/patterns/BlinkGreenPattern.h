#pragma once

#include "IPattern.h"

namespace pflash {

class BlinkGreenPattern final : public IPattern {
public:
    const Frame* frames()         const override;
    size_t       frameCount()     const override;
    const char*  name()           const override;
    uint8_t      maxChannelUsed() const override;
};

} // namespace pflash
