#pragma once

#include "Flasher.h"
#include "LightSensor.h"
#include "ModeManager.h"
#include "PhaseTimer.h"
#include "TrafficLightFsm.h"
#include "inputs/IController.h"

#include <cstdint>

namespace pflash {

// Owns the runtime glue between Flasher, TrafficLightFsm, PhaseTimer, the two
// input controllers, and the LightSensor. Multiplexes three day/night sources
// (Timer, Button, LightSensor) with runtime switching via BOOT long-press.
class TrafficLightCoordinator {
public:
    enum class NightSource : uint8_t { Timer, Button, LightSensor };

    struct Config {
        Flasher*             flasher;
        ModeManager*         modes;
        TrafficLightFsm*     fsm;
        PhaseTimer*          phase;
        IController*         pedButton;
        IController*         bootButton;
        pflash::LightSensor* lightSensor;
        NightSource          initialSource = NightSource::Timer;
    };

    explicit TrafficLightCoordinator(const Config& cfg);

    void begin(uint32_t now_ms);
    void tick(uint32_t now_ms);

private:
    void pollPedestrian(uint32_t now_ms);
    void pollBootButton(uint32_t now_ms);
    void pollPhase(uint32_t now_ms);
    void pollLightSensor(uint32_t now_ms);

    void toggleNight(uint32_t now_ms);
    void cycleSource(uint32_t now_ms);
    void forceSyncToSource(uint32_t now_ms);
    void logSource() const;

    static const char* sourceName(NightSource s);

    Flasher*             flasher_;
    ModeManager*         modes_;
    TrafficLightFsm*     fsm_;
    PhaseTimer*          phase_;
    IController*         pedButton_;
    IController*         bootButton_;
    pflash::LightSensor* ldr_;
    NightSource          activeSource_;
};

} // namespace pflash
