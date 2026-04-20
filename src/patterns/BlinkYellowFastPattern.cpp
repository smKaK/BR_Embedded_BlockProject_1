#include "patterns/BlinkYellowFastPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 250, 0b010 },
    { 250, 0b000 },
};
} // namespace

const Frame* BlinkYellowFastPattern::frames() const {
    return kFrames;
}

size_t BlinkYellowFastPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* BlinkYellowFastPattern::name() const {
    return "BlinkYellowFast";
}

uint8_t BlinkYellowFastPattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
