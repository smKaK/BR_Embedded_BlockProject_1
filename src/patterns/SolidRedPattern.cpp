#include "patterns/SolidRedPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 1000, 0b001 },
};
} // namespace

const Frame* SolidRedPattern::frames() const {
    return kFrames;
}

size_t SolidRedPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* SolidRedPattern::name() const {
    return "SolidRed";
}

uint8_t SolidRedPattern::maxChannelUsed() const {
    return 0;
}

} // namespace pflash
