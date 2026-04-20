#pragma once

#include <cstdint>

namespace pflash {

// Polled photoresistor (LDR) source. Converts the analog reading into a
// hysteresis-guarded Day/Night signal. Not an IController because its events
// are direction-aware (WentDay / WentNight), not button-ish edges.
//
// Wiring assumption: reading >= darkThreshold means "dark". brightThreshold
// must be strictly less than darkThreshold to form the hysteresis band.
class LightSensor {
public:
    enum class Reading : uint8_t { Unknown, Day, Night };
    enum class Event   : uint8_t { None, WentDay, WentNight };

    struct Config {
        uint8_t  pin;
        uint16_t darkThreshold;
        uint16_t brightThreshold;
        uint32_t sampleIntervalMs;
    };

    explicit LightSensor(const Config& c);

    void  begin();
    Event poll(uint32_t now_ms);

    Reading current() const { return reading_; }

private:
    Config   cfg_;
    Reading  reading_{Reading::Unknown};
    uint32_t lastSample_ms_{0};
};

} // namespace pflash
