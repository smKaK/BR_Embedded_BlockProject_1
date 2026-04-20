#include <Arduino.h>

#include "Flasher.h"
#include "FlasherBuilder.h"
#include "ModeManager.h"
#include "PatternId.h"
#include "PhaseTimer.h"
#include "TrafficLightFsm.h"
#include "traffic_light_config.h"
#include "outputs/MultiPinOutput.h"
#include "patterns/BlinkGreenPattern.h"
#include "patterns/BlinkYellowFastPattern.h"
#include "patterns/BlinkYellowPattern.h"
#include "patterns/SolidGreenPattern.h"
#include "patterns/SolidRedPattern.h"
#include "patterns/SolidYellowPattern.h"

#include "debounce/Debouncer.h"
#include "debounce/HysteresisDebounce.h"
#include "inputs/ButtonController.h"
#include "inputs/InterruptSampler.h"

namespace {
using namespace pflash;

MultiPinOutput      gOutput({ kPinRed, kPinYellow, kPinGreen });

SolidRedPattern         gRed;
SolidYellowPattern      gYellow;
SolidGreenPattern       gGreen;
BlinkYellowPattern      gBlinkYellow;
BlinkGreenPattern       gBlinkGreen;
BlinkYellowFastPattern  gBlinkYellowFast;

Flasher gFlasher = FlasherBuilder()
    .withOutput(&gOutput)
    .withPattern(&gRed)
    .build();

ModeManager gModes({
    .flasher  = &gFlasher,
    .patterns = { &gRed, &gYellow, &gGreen, &gBlinkYellow, &gBlinkGreen, &gBlinkYellowFast },
});

TrafficLightFsm gFsm({ .modes = &gModes });

HysteresisDebounce gPedAlgo (kDebounceMs);
HysteresisDebounce gBootAlgo(kDebounceMs);
Debouncer          gPedDeb  (&gPedAlgo);
Debouncer          gBootDeb (&gBootAlgo);

InterruptSampler   gPedSampler;
InterruptSampler   gBootSampler;

ButtonController gPedBtn(ButtonController::Config{
    .pin       = kPinPedButton,
    .debouncer = &gPedDeb,
    .sampler   = &gPedSampler,
    .activeLow = true,
    .pressCmd  = Command::PedestrianPress,
    .label     = "ped",
});

ButtonController gBootBtn(ButtonController::Config{
    .pin       = kPinBootButton,
    .debouncer = &gBootDeb,
    .sampler   = &gBootSampler,
    .activeLow = true,
    .pressCmd  = Command::NightToggle,
    .label     = "boot",
});

PhaseTimer gPhase({ .dayMs = kDayPhaseMs, .nightMs = kNightPhaseMs });

void dispatchControllerEvent(const ControlEvent& ev, uint32_t now_ms) {
    switch (ev.cmd) {
        case Command::PedestrianPress:
            gFsm.dispatch(PedestrianPress{}, now_ms);
            break;
        case Command::NightToggle:
            if (gFsm.in<NightBlink>()) {
                gFsm.dispatch(NightModeOff{}, now_ms);
            } else {
                gFsm.dispatch(NightModeOn{}, now_ms);
            }
            break;
        default:
            break;
    }
}

} // namespace

void setup() {
    Serial.begin(kSerialBaud);
    delay(50);

    gModes.begin();
    gPedBtn.begin();
    gBootBtn.begin();

    const uint32_t now = millis();
    gPhase.begin(now);
    gFsm.begin(DayRed{}, now);
}

void loop() {
    const uint32_t now = millis();

    gFlasher.update();

    dispatchControllerEvent(gPedBtn.poll(),  now);
    dispatchControllerEvent(gBootBtn.poll(), now);

    switch (gPhase.tick(now)) {
        case PhaseTimer::Flip::ToNight: gFsm.dispatch(NightModeOn{},  now); break;
        case PhaseTimer::Flip::ToDay:   gFsm.dispatch(NightModeOff{}, now); break;
        case PhaseTimer::Flip::None:    break;
    }

    gFsm.tick(now);
}
