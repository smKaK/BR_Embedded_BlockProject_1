#include <Arduino.h>

#include "Flasher.h"
#include "FlasherBuilder.h"
#include "outputs/MultiPinOutput.h"
#include "patterns/AlternatingPattern.h"
#include "patterns/DoubleBlinkPolicePattern.h"
#include "patterns/SosPattern.h"

namespace {

constexpr uint8_t  kPinRed      = 4;
constexpr uint8_t  kPinBlue     = 5;
constexpr uint32_t kSerialBaud  = 115200;

pflash::MultiPinOutput            gOutput({ kPinRed, kPinBlue });
pflash::AlternatingPattern        gAlternating;
pflash::DoubleBlinkPolicePattern  gDoubleBlink;
pflash::SosPattern                gSos;

pflash::Flasher gFlasher = pflash::FlasherBuilder()
    .withOutput(&gOutput)
    .withPattern(&gDoubleBlink)
    .build();

void printHelp() {
    Serial.println(F("[pflash] commands:"));
    Serial.println(F("  p0  -> Alternating"));
    Serial.println(F("  p1  -> DoubleBlinkPolice (default)"));
    Serial.println(F("  p2  -> SOS"));
    Serial.println(F("  ?   -> print current pattern"));
}

void handleSerialCommand(const String& line) {
    if (line.length() == 0) {
        return;
    }

    if (line == "p0") {
        gFlasher.setPattern(&gAlternating);
        Serial.printf("[pflash] switched -> %s\n", gFlasher.currentPatternName());
    } else if (line == "p1") {
        gFlasher.setPattern(&gDoubleBlink);
        Serial.printf("[pflash] switched -> %s\n", gFlasher.currentPatternName());
    } else if (line == "p2") {
        gFlasher.setPattern(&gSos);
        Serial.printf("[pflash] switched -> %s\n", gFlasher.currentPatternName());
    } else if (line == "?") {
        Serial.printf("[pflash] current pattern: %s\n", gFlasher.currentPatternName());
    } else {
        Serial.printf("[pflash] unknown cmd: '%s'\n", line.c_str());
        printHelp();
    }
}

void pumpSerial() {
    static String buf;
    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            buf.trim();
            handleSerialCommand(buf);
            buf = "";
            continue;
        }
        if (buf.length() < 32) {
            buf += c;
        }
    }
}

} // namespace

void setup() {
    Serial.begin(kSerialBaud);
    delay(50);

    gFlasher.begin();

    Serial.printf("[pflash] started pattern=%s channels=2 (GPIO %u red, GPIO %u blue)\n",
                  gFlasher.currentPatternName(),
                  static_cast<unsigned>(kPinRed),
                  static_cast<unsigned>(kPinBlue));
    printHelp();
}

void loop() {
    gFlasher.update();
    pumpSerial();
}
