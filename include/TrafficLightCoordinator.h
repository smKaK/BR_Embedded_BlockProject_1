#pragma once

#include "Flasher.h"
#include "ModeManager.h"
#include "PhaseTimer.h"
#include "TrafficLightFsm.h"
#include "inputs/IController.h"

#include <cstdint>

namespace pflash {

// Owns the runtime glue between Flasher, TrafficLightFsm, PhaseTimer, and the
// two input controllers. Translates ControlEvents and PhaseTimer::Flip values
// into FSM-specific events so main.cpp stays pure wiring.
class TrafficLightCoordinator {
public:
    struct Config {
        Flasher*         flasher;
        ModeManager*     modes;
        TrafficLightFsm* fsm;
        PhaseTimer*      phase;
        IController*     pedButton;
        IController*     bootButton;
    };

    explicit TrafficLightCoordinator(const Config& cfg)
        : flasher_(cfg.flasher),
          modes_(cfg.modes),
          fsm_(cfg.fsm),
          phase_(cfg.phase),
          pedButton_(cfg.pedButton),
          bootButton_(cfg.bootButton) {}

    void begin(uint32_t now_ms) {
        modes_->begin();
        pedButton_->begin();
        bootButton_->begin();
        phase_->begin(now_ms);
        fsm_->begin(DayRed{}, now_ms);
    }

    void tick(uint32_t now_ms) {
        flasher_->update();

        dispatchControllerEvent(pedButton_->poll(),  now_ms);
        dispatchControllerEvent(bootButton_->poll(), now_ms);

        switch (phase_->tick(now_ms)) {
            case PhaseTimer::Flip::ToNight: fsm_->dispatch(NightModeOn{},  now_ms); break;
            case PhaseTimer::Flip::ToDay:   fsm_->dispatch(NightModeOff{}, now_ms); break;
            case PhaseTimer::Flip::None:                                             break;
        }

        fsm_->tick(now_ms);
    }

private:
    void dispatchControllerEvent(const ControlEvent& ev, uint32_t now_ms) {
        switch (ev.cmd) {
            case Command::PedestrianPress:
                fsm_->dispatch(PedestrianPress{}, now_ms);
                break;
            case Command::NightToggle:
                if (fsm_->in<NightBlink>()) {
                    fsm_->dispatch(NightModeOff{}, now_ms);
                } else {
                    fsm_->dispatch(NightModeOn{}, now_ms);
                }
                break;
            default:
                break;
        }
    }

    Flasher*         flasher_;
    ModeManager*     modes_;
    TrafficLightFsm* fsm_;
    PhaseTimer*      phase_;
    IController*     pedButton_;
    IController*     bootButton_;
};

} // namespace pflash
