#include "inputs/ButtonController.h"

#include <Arduino.h>

namespace pflash {

ButtonController::ButtonController(const Config& cfg)
    : pin_(cfg.pin), deb_(cfg.debouncer), sampler_(cfg.sampler),
      activeLow_(cfg.activeLow), pressCmd_(cfg.pressCmd), label_(cfg.label) {}

void ButtonController::begin() {
    pinMode(pin_, activeLow_ ? INPUT_PULLUP : INPUT);
    if (sampler_) {
        sampler_->begin(pin_, activeLow_);
    }
    const bool raw     = (digitalRead(pin_) == HIGH);
    const bool pressed = activeLow_ ? !raw : raw;
    if (deb_) {
        deb_->begin(pressed, millis());
    }
}

ControlEvent ButtonController::poll() {
    if (!deb_) {
        return { Command::None, 0 };
    }
    const uint32_t now           = millis();
    const bool     samplerSays   = !sampler_ || sampler_->shouldSample(now);
    const bool     debouncerWants = deb_->pending();
    if (!samplerSays && !debouncerWants) {
        return { Command::None, 0 };
    }
    const bool raw     = (digitalRead(pin_) == HIGH);
    const bool pressed = activeLow_ ? !raw : raw;
    const auto edge    = deb_->update(pressed, now);
    if (edge == Debouncer::Edge::Rising) {
        return { pressCmd_, 0 };
    }
    return { Command::None, 0 };
}

} // namespace pflash
