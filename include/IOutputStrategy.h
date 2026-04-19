#pragma once

#include <cstdint>

namespace pflash {

class IOutputStrategy {
public:
    virtual ~IOutputStrategy() = default;

    virtual void    begin()                         = 0;
    virtual void    setChannel(uint8_t i, bool on)  = 0;
    virtual uint8_t channelCount() const            = 0;
};

} // namespace pflash
