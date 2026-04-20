#pragma once

#include "Flasher.h"
#include "IPattern.h"
#include "PatternId.h"

#include <initializer_list>

namespace pflash {

// Owns the pattern registry and forwards applyPattern() calls to the Flasher.
// Input dispatch now lives in main.cpp (controllers poll -> FSM dispatch).
class ModeManager {
public:
    static constexpr uint8_t kMaxPatterns = 8;

    struct Config {
        Flasher*                         flasher;
        std::initializer_list<IPattern*> patterns;
    };

    explicit ModeManager(const Config& cfg);

    void begin();

    void applyPattern(uint8_t idx, const char* source);
    void applyPattern(PatternId p, const char* source);

    uint8_t     currentIndex()       const { return idx_; }
    uint8_t     patternCount()       const { return patternCount_; }
    const char* currentPatternName() const { return flasher_ ? flasher_->currentPatternName() : ""; }

private:
    Flasher*  flasher_;
    IPattern* patterns_[kMaxPatterns]{};
    uint8_t   patternCount_{0};
    uint8_t   idx_{0};
};

} // namespace pflash
