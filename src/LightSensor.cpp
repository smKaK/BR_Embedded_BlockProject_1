#include "LightSensor.h"

#include <Arduino.h>

namespace pflash {

LightSensor::LightSensor(const Config& c) : cfg_(c) {}

void LightSensor::begin() {
    pinMode(cfg_.pin, INPUT);
}

LightSensor::Event LightSensor::poll(uint32_t now_ms) {
    if (lastSample_ms_ != 0 &&
        (uint32_t)(now_ms - lastSample_ms_) < cfg_.sampleIntervalMs) {
        return Event::None;
    }
    lastSample_ms_ = now_ms == 0 ? 1 : now_ms;

    const int v = analogRead(cfg_.pin);
    if (v >= cfg_.darkThreshold && reading_ != Reading::Night) {
        reading_ = Reading::Night;
        return Event::WentNight;
    }
    if (v <= cfg_.brightThreshold && reading_ != Reading::Day) {
        reading_ = Reading::Day;
        return Event::WentDay;
    }
    return Event::None;
}

} // namespace pflash
