#include "patterns/SosPattern.h"

namespace pflash {

namespace {
// Morse SOS on both channels together.
// Unit = 200 ms.  dot = 1u, dash = 3u,
// intra-char gap = 1u, inter-char gap = 3u, end-of-word gap = 7u.
constexpr Frame kFrames[] = {
    // S : . . .
    { 200, 0b11 }, { 200, 0b00 },
    { 200, 0b11 }, { 200, 0b00 },
    { 200, 0b11 },
    { 600, 0b00 },                 // inter-char gap
    // O : - - -
    { 600, 0b11 }, { 200, 0b00 },
    { 600, 0b11 }, { 200, 0b00 },
    { 600, 0b11 },
    { 600, 0b00 },                 // inter-char gap
    // S : . . .
    { 200, 0b11 }, { 200, 0b00 },
    { 200, 0b11 }, { 200, 0b00 },
    { 200, 0b11 },
    { 1400, 0b00 },                // end-of-word gap before repeat
};
} // namespace

const Frame* SosPattern::frames() const {
    return kFrames;
}

size_t SosPattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* SosPattern::name() const {
    return "SOS";
}

uint8_t SosPattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
