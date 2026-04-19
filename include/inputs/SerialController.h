#pragma once

#include "IController.h"

#include <Arduino.h>

namespace pflash {

class SerialController final : public IController {
public:
    SerialController() = default;

    void         begin()         override;
    ControlEvent poll()          override;
    const char*  name() const    override { return "serial"; }

private:
    static constexpr uint8_t kMaxLine = 32;
    String buf_;
};

} // namespace pflash
