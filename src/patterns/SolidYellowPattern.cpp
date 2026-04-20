#include "patterns/SolidYellowPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 1000, 0b010 },
};
} // namespace

const Frame* SolidYellowPattern::frames() const {
    return kFrames;
}

size_t SolidYellowPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* SolidYellowPattern::name() const {
    return "SolidYellow";
}

uint8_t SolidYellowPattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
