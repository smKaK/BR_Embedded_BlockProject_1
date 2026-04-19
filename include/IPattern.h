#pragma once

#include <cstdint>
#include <cstddef>

namespace pflash {

struct Frame {
    uint32_t duration_ms;
    uint32_t channel_mask;
};

class IPattern {
public:
    virtual ~IPattern() = default;

    virtual const Frame* frames()         const = 0;
    virtual size_t       frameCount()     const = 0;
    virtual const char*  name()           const = 0;
    virtual uint8_t      maxChannelUsed() const = 0;
};

} // namespace pflash
