#pragma once

#include "IOutputStrategy.h"
#include "IPattern.h"

#include <cstddef>
#include <cstdint>

namespace pflash {

class IFlasherObserver {
public:
    virtual ~IFlasherObserver() = default;
    virtual void onFrameChange(size_t /*idx*/, uint32_t /*mask*/) {}
    virtual void onPatternChange(const char* /*name*/) {}
};

class Flasher {
public:
    Flasher(IOutputStrategy* out, IPattern* pat);

    Flasher(const Flasher&)            = delete;
    Flasher& operator=(const Flasher&) = delete;

    void begin();
    void update();

    void setPattern(IPattern* p);
    void setObserver(IFlasherObserver* o);

    const char* currentPatternName() const;

private:
    void applyMask(uint32_t mask);

    IOutputStrategy*  out_{nullptr};
    IPattern*         pat_{nullptr};
    IFlasherObserver* obs_{nullptr};
    size_t            frameIdx_{0};
    uint32_t          tLast_{0};
};

} // namespace pflash
