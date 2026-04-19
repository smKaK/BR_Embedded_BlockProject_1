#include "inputs/SerialController.h"

#include "PatternId.h"

namespace pflash {

void SerialController::begin() {
    buf_.reserve(kMaxLine);
}

ControlEvent SerialController::poll() {
    while (Serial.available() > 0) {
        const char c = static_cast<char>(Serial.read());
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            buf_.trim();
            String line = buf_;
            buf_ = "";

            if (line.length() == 0) {
                return { Command::None, 0 };
            }
            if (line == "p0") { return { Command::SelectPattern, idx(PatternId::Alternating)       }; }
            if (line == "p1") { return { Command::SelectPattern, idx(PatternId::DoubleBlinkPolice) }; }
            if (line == "p2") { return { Command::SelectPattern, idx(PatternId::SOS)               }; }
            if (line == "?")  { return { Command::PrintCurrent,  0                                 }; }
            return { Command::PrintHelp, 0 };
        }
        if (buf_.length() < kMaxLine) {
            buf_ += c;
        }
    }
    return { Command::None, 0 };
}

} // namespace pflash
