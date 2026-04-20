#include "inputs/ButtonController.h"

#include <Arduino.h>

namespace pflash {

ButtonController::ButtonController(const Config& cfg)
    : pin_(cfg.pin), deb_(cfg.debouncer), sampler_(cfg.sampler),
      activeLow_(cfg.activeLow), pressCmd_(cfg.pressCmd), label_(cfg.label),
      holdMs_(cfg.holdMs), holdCmd_(cfg.holdCmd) {}

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
    const uint32_t now            = millis();
    const bool     samplerSays    = !sampler_ || sampler_->shouldSample(now);
    const bool     debouncerWants = deb_->pending();
    const bool     holdPending    = (holdMs_ > 0) && (pressStart_ms_ != 0);

    if (!samplerSays && !debouncerWants && !holdPending) {
        return { Command::None, 0 };
    }

    Debouncer::Edge edge = Debouncer::Edge::None;
    if (samplerSays || debouncerWants) {
        const bool raw     = (digitalRead(pin_) == HIGH);
        const bool pressed = activeLow_ ? !raw : raw;
        edge               = deb_->update(pressed, now);
    }

    // Legacy mode: no hold detection, emit on rising edge.
    if (holdMs_ == 0) {
        if (edge == Debouncer::Edge::Rising) {
            return { pressCmd_, 0 };
        }
        return { Command::None, 0 };
    }

    // Hold-aware mode.
    if (edge == Debouncer::Edge::Rising) {
        pressStart_ms_ = now == 0 ? 1 : now;  // 0 is reserved for "not pressed"
        holdFired_     = false;
        return { Command::None, 0 };
    }

    if (edge == Debouncer::Edge::Falling) {
        const bool wasHold = holdFired_;
        pressStart_ms_     = 0;
        holdFired_         = false;
        if (!wasHold) {
            return { pressCmd_, 0 };
        }
        return { Command::None, 0 };
    }

    // Still held, no edge.
    if (pressStart_ms_ != 0 && !holdFired_ &&
        (uint32_t)(now - pressStart_ms_) >= holdMs_) {
        holdFired_ = true;
        return { holdCmd_, 0 };
    }

    return { Command::None, 0 };
}

} // namespace pflash
