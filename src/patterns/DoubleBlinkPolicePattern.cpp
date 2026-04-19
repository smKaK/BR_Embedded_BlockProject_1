#include "patterns/DoubleBlinkPolicePattern.h"

namespace pflash {

namespace {
constexpr Frame kFrames[] = {
    {  70, 0b01 },  // red on
    {  70, 0b00 },  // gap
    {  70, 0b01 },  // red on
    {  70, 0b00 },  // gap
    { 100, 0b00 },  // inter-color pause
    {  70, 0b10 },  // blue on
    {  70, 0b00 },  // gap
    {  70, 0b10 },  // blue on
    {  70, 0b00 },  // gap
    { 100, 0b00 },  // inter-color pause
};
} // namespace

const Frame* DoubleBlinkPolicePattern::frames() const {
    return kFrames;
}

size_t DoubleBlinkPolicePattern::frameCount() const {
    return sizeof(kFrames) / sizeof(kFrames[0]);
}

const char* DoubleBlinkPolicePattern::name() const {
    return "DoubleBlinkPolice";
}

uint8_t DoubleBlinkPolicePattern::maxChannelUsed() const {
    return 1;
}

} // namespace pflash
