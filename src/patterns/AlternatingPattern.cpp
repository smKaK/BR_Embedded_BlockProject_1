#include "patterns/AlternatingPattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    { 250, 0b01 },  // red only
    { 250, 0b10 },  // blue only
};
} // namespace

const Frame* AlternatingPattern::frames() const {
    return kFrames;
}

size_t AlternatingPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* AlternatingPattern::name() const {
    return "Alternating";
}

uint8_t AlternatingPattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
