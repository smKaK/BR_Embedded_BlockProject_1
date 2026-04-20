#include "patterns/BlinkYellowPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 500, 0b010 },
    { 500, 0b000 },
};
} // namespace

const Frame* BlinkYellowPattern::frames() const {
    return kFrames;
}

size_t BlinkYellowPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* BlinkYellowPattern::name() const {
    return "BlinkYellow";
}

uint8_t BlinkYellowPattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
