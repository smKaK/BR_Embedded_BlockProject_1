#include "ModeManager.h"

#include <Arduino.h>

namespace pflash {

ModeManager::ModeManager(const Config& cfg) : flasher_(cfg.flasher) {
    for (auto* p : cfg.patterns) {
        if (p && patternCount_ < kMaxPatterns) {
            patterns_[patternCount_++] = p;
        }
    }
}

void ModeManager::begin() {
    if (!flasher_ || patternCount_ == 0) {
        Serial.println(F("[pflash][FATAL] ModeManager misconfigured"));
        while (true) { delay(1000); }
    }
    flasher_->begin();
    flasher_->setPattern(patterns_[idx_]);
}

void ModeManager::applyPattern(PatternId p, const char* source) {
    applyPattern(idx(p), source);
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

} // namespace pflash
