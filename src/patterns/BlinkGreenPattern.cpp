#include "patterns/BlinkGreenPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 250, 0b100 },
    { 250, 0b000 },
};
} // namespace

const Frame* BlinkGreenPattern::frames() const {
    return kFrames;
}

size_t BlinkGreenPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* BlinkGreenPattern::name() const {
    return "BlinkGreen";
}

uint8_t BlinkGreenPattern::maxChannelUsed() const {
    return 2;
}

} // namespace pflash
