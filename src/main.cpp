#include <Arduino.h>

#include "Flasher.h"
#include "FlasherBuilder.h"
#include "ModeManager.h"
#include "PatternId.h"
#include "outputs/MultiPinOutput.h"
#include "patterns/AlternatingPattern.h"
#include "patterns/DoubleBlinkPolicePattern.h"
#include "patterns/SosPattern.h"

#include "debounce/HysteresisDebounce.h"
#include "debounce/Debouncer.h"
#include "inputs/ButtonController.h"
#include "inputs/InterruptSampler.h"
#include "inputs/PollingSampler.h"
#include "inputs/SerialController.h"

namespace {

constexpr uint8_t  kPinRed        = 4;
constexpr uint8_t  kPinBlue       = 5;
constexpr uint8_t  kPinExtButton  = 2;
constexpr uint8_t  kPinBootButton = 0;
constexpr uint32_t kSerialBaud    = 115200;
constexpr uint32_t kDebounceMs    = 20;

pflash::MultiPinOutput            gOutput({ kPinRed, kPinBlue });
pflash::AlternatingPattern        gAlternating;
pflash::DoubleBlinkPolicePattern  gDoubleBlink;
pflash::SosPattern                gSos;

pflash::Flasher gFlasher = pflash::FlasherBuilder()
    .withOutput(&gOutput)
    .withPattern(&gDoubleBlink)
    .build();

pflash::HysteresisDebounce gExtAlgo (kDebounceMs);
pflash::HysteresisDebounce gBootAlgo(kDebounceMs);
pflash::Debouncer          gExtDeb  (&gExtAlgo);
pflash::Debouncer          gBootDeb (&gBootAlgo);

pflash::SerialController  gSerialCtl;
pflash::InterruptSampler  gExtSampler;       // external button reacts on interrupt
pflash::PollingSampler    gBootSampler;      // BOOT button stays polled

pflash::ButtonController gExtBtn({
    .pin       = kPinExtButton,
    .debouncer = &gExtDeb,
    .sampler   = &gExtSampler,
    .label     = "ext",
});

pflash::ButtonController gBootBtn({
    .pin       = kPinBootButton,
    .debouncer = &gBootDeb,
    .sampler   = &gBootSampler,
    .label     = "boot",
});

pflash::ModeManager gModes({
    .flasher      = &gFlasher,
    .patterns     = { &gAlternating, &gDoubleBlink, &gSos },
    .controllers  = { &gSerialCtl, &gExtBtn, &gBootBtn },
    .defaultIndex = pflash::idx(pflash::PatternId::DoubleBlinkPolice),
});

} // namespace

void setup() {
    Serial.begin(kSerialBaud);
    delay(50);

    gModes.begin();
    gModes.printHelp();
}

void loop() {
    gFlasher.update();
    gModes.tick();
}
