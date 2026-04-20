#pragma once

#include "ModeManager.h"
#include "PatternId.h"
#include "fsm.hpp"
#include "traffic_light_config.h"

namespace pflash {

struct DayRed         {};
struct DayGreen       {};
struct DayGreenBlink  {};
struct DayYellow      {};
struct DayYellowBlink {};
struct NightBlink     {};

struct PedestrianPress {};
struct NightModeOn     {};
struct NightModeOff    {};

class TrafficLightFsm
    : public fsm::StateMachine<TrafficLightFsm,
                               DayRed, DayGreen, DayGreenBlink,
                               DayYellow, DayYellowBlink,
                               NightBlink> {
public:
    struct Config {
        ModeManager* modes;
    };

    explicit TrafficLightFsm(const Config& cfg) : modes_(cfg.modes) {}

    void on_enter(DayRed&) {
        modes_->applyPattern(PatternId::SolidRed, "fsm");
        set_timeout(kDayRedMs);
    }
    void on_enter(DayGreen&) {
        modes_->applyPattern(PatternId::SolidGreen, "fsm");
        set_timeout(kDayGreenMs);
    }
    void on_enter(DayGreenBlink&) {
        modes_->applyPattern(PatternId::BlinkGreen, "fsm");
        set_timeout(kDayGreenBlinkMs);
    }
    void on_enter(DayYellow&) {
        modes_->applyPattern(PatternId::SolidYellow, "fsm");
        set_timeout(kDayYellowMs);
    }
    void on_enter(DayYellowBlink&) {
        modes_->applyPattern(PatternId::BlinkYellowFast, "fsm");
        set_timeout(kDayYellowBlinkMs);
    }
    void on_enter(NightBlink&) {
        // BlinkYellow's own 2-frame table drives the blink timing; the FSM
        // stays parked in NightBlink until NightModeOff arrives.
        modes_->applyPattern(PatternId::BlinkYellow, "fsm");
    }

    Trans handle(DayRed&,         const fsm::Timeout&) { return Trans::to<DayGreen>();       }
    Trans handle(DayGreen&,       const fsm::Timeout&) { return Trans::to<DayGreenBlink>();  }
    Trans handle(DayGreenBlink&,  const fsm::Timeout&) { return Trans::to<DayYellow>();      }
    Trans handle(DayYellow&,      const fsm::Timeout&) { return Trans::to<DayYellowBlink>(); }
    Trans handle(DayYellowBlink&, const fsm::Timeout&) { return Trans::to<DayRed>();         }

    // PedestrianPress is accepted only in DayRed — during any other phase a
    // press must not shorten the cycle mid-phase.
    Trans handle(DayRed&, const PedestrianPress&) { return Trans::to<DayGreen>(); }

    Trans handle(DayRed&,         const NightModeOn&) { return Trans::to<NightBlink>(); }
    Trans handle(DayGreen&,       const NightModeOn&) { return Trans::to<NightBlink>(); }
    Trans handle(DayGreenBlink&,  const NightModeOn&) { return Trans::to<NightBlink>(); }
    Trans handle(DayYellow&,      const NightModeOn&) { return Trans::to<NightBlink>(); }
    Trans handle(DayYellowBlink&, const NightModeOn&) { return Trans::to<NightBlink>(); }

    Trans handle(NightBlink&, const NightModeOff&) { return Trans::to<DayRed>(); }

    template <class S, class E>
    Trans handle(S&, const E&) { return Trans::stay(); }

private:
    ModeManager* modes_;
};

} // namespace pflash
