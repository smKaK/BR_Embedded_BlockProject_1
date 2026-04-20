#include "patterns/SolidGreenPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 1000, 0b100 },
};
} // namespace

const Frame* SolidGreenPattern::frames() const {
    return kFrames;
}

size_t SolidGreenPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* SolidGreenPattern::name() const {
    return "SolidGreen";
}

uint8_t SolidGreenPattern::maxChannelUsed() const {
    return 2;
}

} // namespace pflash
