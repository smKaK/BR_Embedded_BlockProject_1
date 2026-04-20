#pragma once

#include "IButtonSampler.h"
#include "IController.h"
#include "debounce/Debouncer.h"

namespace pflash {

class ButtonController final : public IController {
public:
    struct Config {
        uint8_t         pin;
        Debouncer*      debouncer;
        IButtonSampler* sampler;
        bool            activeLow = true;
        Command         pressCmd  = Command::None;
        const char*     label     = "btn";
        uint32_t        holdMs    = 0;              // 0 disables hold detection
        Command         holdCmd   = Command::None;
    };

    explicit ButtonController(const Config& cfg);

    void         begin()         override;
    ControlEvent poll()          override;
    const char*  name() const    override { return label_; }

private:
    uint8_t         pin_;
    Debouncer*      deb_;
    IButtonSampler* sampler_;
    bool            activeLow_;
    Command         pressCmd_;
    const char*     label_;
    uint32_t        holdMs_;
    Command         holdCmd_;
    uint32_t        pressStart_ms_{0};
    bool            holdFired_{false};
};

} // namespace pflash
