#include "ModeManager.h"

#include <Arduino.h>

namespace pflash {

ModeManager::ModeManager(const Config& cfg)
    : flasher_(cfg.flasher), idx_(cfg.defaultIndex) {
    for (auto* p : cfg.patterns) {
        if (p && patternCount_ < kMaxPatterns) {
            patterns_[patternCount_++] = p;
        }
    }
    for (auto* c : cfg.controllers) {
        if (c && controllerCount_ < kMaxControllers) {
            controllers_[controllerCount_++] = c;
        }
    }
}

void ModeManager::begin() {
    if (!flasher_ || patternCount_ == 0) {
        Serial.println(F("[pflash][FATAL] ModeManager misconfigured"));
        while (true) { delay(1000); }
    }
    if (idx_ >= patternCount_) {
        idx_ = 0;
    }
    flasher_->begin();
    flasher_->setPattern(patterns_[idx_]);

    for (uint8_t i = 0; i < controllerCount_; ++i) {
        controllers_[i]->begin();
    }
}

void ModeManager::tick() {
    for (uint8_t i = 0; i < controllerCount_; ++i) {
        const ControlEvent ev = controllers_[i]->poll();
        if (ev.cmd != Command::None) {
            dispatch(ev, controllers_[i]->name());
        }
    }
}

void ModeManager::cycleNext(const char* source) {
    if (patternCount_ == 0) { return; }
    applyPattern((idx_ + 1) % patternCount_, source);
}

void ModeManager::applyPattern(uint8_t idx, const char* source) {
    if (idx >= patternCount_) {
        Serial.printf("[pflash][%s] bad pattern idx %u\n",
                      source ? source : "?", static_cast<unsigned>(idx));
        return;
    }
    idx_ = idx;
    flasher_->setPattern(patterns_[idx_]);
    Serial.printf("[pflash][%s] switched -> %s\n",
                  source ? source : "?", flasher_->currentPatternName());
}

void ModeManager::printCurrent() const {
    Serial.printf("[pflash] current pattern [%u]: %s\n",
                  static_cast<unsigned>(idx_), flasher_->currentPatternName());
}

void ModeManager::printHelp() const {
    Serial.println(F("[pflash] commands:"));
    for (uint8_t i = 0; i < patternCount_; ++i) {
        Serial.printf("  p%u  -> %s%s\n",
                      static_cast<unsigned>(i),
                      patterns_[i]->name(),
                      (i == idx_) ? " (current)" : "");
    }
    Serial.println(F("  ?   -> print current pattern"));
    Serial.println(F("  (button press: cycle next)"));
}

void ModeManager::dispatch(const ControlEvent& ev, const char* source) {
    switch (ev.cmd) {
        case Command::NextPattern:   cycleNext(source);            break;
        case Command::SelectPattern: applyPattern(ev.arg, source); break;
        case Command::PrintCurrent:  printCurrent();               break;
        case Command::PrintHelp:     printHelp();                  break;
        case Command::None:                                        break;
    }
}

} // namespace pflash
