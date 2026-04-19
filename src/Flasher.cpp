#include "Flasher.h"

#include <Arduino.h>

namespace pflash {

Flasher::Flasher(IOutputStrategy* out, IPattern* pat)
    : out_(out), pat_(pat) {}

void Flasher::begin() {
    if (!out_ || !pat_) {
        Serial.println(F("[pflash][FATAL] missing output or pattern"));
        while (true) { delay(1000); }
    }
    if (pat_->maxChannelUsed() >= out_->channelCount()) {
        Serial.printf("[pflash][FATAL] pattern '%s' uses ch %u but output has %u channel(s)\n",
                      pat_->name(),
                      static_cast<unsigned>(pat_->maxChannelUsed()),
                      static_cast<unsigned>(out_->channelCount()));
        while (true) { delay(1000); }
    }

    out_->begin();
    frameIdx_ = 0;
    tLast_    = millis();
    applyMask(pat_->frames()[frameIdx_].channel_mask);

    if (obs_) {
        obs_->onPatternChange(pat_->name());
        obs_->onFrameChange(frameIdx_, pat_->frames()[frameIdx_].channel_mask);
    }
}

void Flasher::update() {
    if (!out_ || !pat_) {
        return;
    }

    const Frame& cur = pat_->frames()[frameIdx_];
    const uint32_t now = millis();
    if ((now - tLast_) < cur.duration_ms) {
        return;
    }

    frameIdx_ = (frameIdx_ + 1) % pat_->frameCount();
    tLast_    = now;
    const uint32_t mask = pat_->frames()[frameIdx_].channel_mask;
    applyMask(mask);

    if (obs_) {
        obs_->onFrameChange(frameIdx_, mask);
    }
}

void Flasher::setPattern(IPattern* p) {
    if (!p) {
        return;
    }
    if (out_ && p->maxChannelUsed() >= out_->channelCount()) {
        Serial.printf("[pflash] refuse pattern '%s': uses ch %u, output has %u\n",
                      p->name(),
                      static_cast<unsigned>(p->maxChannelUsed()),
                      static_cast<unsigned>(out_->channelCount()));
        return;
    }
    pat_      = p;
    frameIdx_ = 0;
    tLast_    = millis();
    applyMask(pat_->frames()[frameIdx_].channel_mask);

    if (obs_) {
        obs_->onPatternChange(pat_->name());
        obs_->onFrameChange(frameIdx_, pat_->frames()[frameIdx_].channel_mask);
    }
}

void Flasher::setObserver(IFlasherObserver* o) {
    obs_ = o;
}

const char* Flasher::currentPatternName() const {
    return pat_ ? pat_->name() : "";
}

void Flasher::applyMask(uint32_t mask) {
    const uint8_t n = out_->channelCount();
    for (uint8_t i = 0; i < n; ++i) {
        out_->setChannel(i, (mask >> i) & 1u);
    }
}

} // namespace pflash
