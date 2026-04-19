#include "outputs/MultiPinOutput.h"

#include <Arduino.h>

namespace pflash {

MultiPinOutput::MultiPinOutput(std::initializer_list<uint8_t> pins, bool activeLow)
    : MultiPinOutput(pins.begin(), static_cast<uint8_t>(pins.size()), activeLow) {}

MultiPinOutput::MultiPinOutput(const uint8_t* pins, uint8_t count, bool activeLow)
    : activeLow_(activeLow) {
    const uint8_t n = (count > kMaxPins) ? kMaxPins : count;
    for (uint8_t i = 0; i < n; ++i) {
        pins_[i] = pins[i];
    }
    count_ = n;
}

void MultiPinOutput::begin() {
    for (uint8_t i = 0; i < count_; ++i) {
        pinMode(pins_[i], OUTPUT);
        digitalWrite(pins_[i], activeLow_ ? HIGH : LOW);
    }
}

void MultiPinOutput::setChannel(uint8_t i, bool on) {
    if (i >= count_) {
        return;
    }
    const bool level = activeLow_ ? !on : on;
    digitalWrite(pins_[i], level ? HIGH : LOW);
}

uint8_t MultiPinOutput::channelCount() const {
    return count_;
}

} // namespace pflash
