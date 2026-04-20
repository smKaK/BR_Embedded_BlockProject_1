#include <Arduino.h>

#include "Flasher.h"
#include "FlasherBuilder.h"
#include "ModeManager.h"
#include "PatternId.h"
#include "PhaseTimer.h"
#include "TrafficLightCoordinator.h"
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

TrafficLightCoordinator gCoord(TrafficLightCoordinator::Config{
    .flasher    = &gFlasher,
    .modes      = &gModes,
    .fsm        = &gFsm,
    .phase      = &gPhase,
    .pedButton  = &gPedBtn,
    .bootButton = &gBootBtn,
});

} // namespace

void setup() {
    Serial.begin(kSerialBaud);
    delay(50);

    gCoord.begin(millis());
}

void loop() {
    gCoord.tick(millis());
}
