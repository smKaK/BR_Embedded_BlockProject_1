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
};

} // namespace pflash
