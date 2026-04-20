#pragma once

#include <cstdint>

namespace pflash {

enum class Command : uint8_t {
    None = 0,
    PedestrianPress,
    NightToggle,
};

struct ControlEvent {
    Command cmd{Command::None};
    uint8_t arg{0};
};

class IController {
public:
    virtual ~IController() = default;

    virtual void         begin()         = 0;
    virtual ControlEvent poll()          = 0;
    virtual const char*  name() const    = 0;
};

} // namespace pflash
