#pragma once

#include "IOutputStrategy.h"

#include <array>
#include <initializer_list>

namespace pflash {

class MultiPinOutput final : public IOutputStrategy {
public:
    static constexpr uint8_t kMaxPins = 8;

    MultiPinOutput(std::initializer_list<uint8_t> pins, bool activeLow = false);
    MultiPinOutput(const uint8_t* pins, uint8_t count, bool activeLow = false);

    void    begin() override;
    void    setChannel(uint8_t i, bool on) override;
    uint8_t channelCount() const override;

private:
    std::array<uint8_t, kMaxPins> pins_{};
    uint8_t count_{0};
    bool    activeLow_{false};
};

} // namespace pflash
