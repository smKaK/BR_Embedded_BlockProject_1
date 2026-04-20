#include "TrafficLightCoordinator.h"

#include <Arduino.h>

namespace pflash {

TrafficLightCoordinator::TrafficLightCoordinator(const Config& cfg)
    : flasher_(cfg.flasher),
      modes_(cfg.modes),
      fsm_(cfg.fsm),
      phase_(cfg.phase),
      pedButton_(cfg.pedButton),
      bootButton_(cfg.bootButton),
      ldr_(cfg.lightSensor),
      activeSource_(cfg.initialSource) {}

void TrafficLightCoordinator::begin(uint32_t now_ms) {
    modes_->begin();
    pedButton_->begin();
    bootButton_->begin();
    ldr_->begin();
    phase_->begin(now_ms);
    fsm_->begin(DayRed{}, now_ms);
    logSource();
}

void TrafficLightCoordinator::tick(uint32_t now_ms) {
    flasher_->update();
    pollPedestrian(now_ms);
    pollBootButton(now_ms);
    pollPhase(now_ms);
    pollLightSensor(now_ms);
    fsm_->tick(now_ms);
}

void TrafficLightCoordinator::pollPedestrian(uint32_t now_ms) {
    const ControlEvent ev = pedButton_->poll();
    if (ev.cmd == Command::PedestrianPress) {
        fsm_->dispatch(PedestrianPress{}, now_ms);
    }
}

void TrafficLightCoordinator::pollBootButton(uint32_t now_ms) {
    const ControlEvent ev = bootButton_->poll();
    if (ev.cmd == Command::NightToggle) {
        if (activeSource_ == NightSource::Button) {
            toggleNight(now_ms);
        }
    } else if (ev.cmd == Command::CycleNightSource) {
        cycleSource(now_ms);
    }
}

void TrafficLightCoordinator::pollPhase(uint32_t now_ms) {
    const auto flip = phase_->tick(now_ms);
    if (activeSource_ != NightSource::Timer) return;
    if (flip == PhaseTimer::Flip::ToNight) fsm_->dispatch(NightModeOn{},  now_ms);
    if (flip == PhaseTimer::Flip::ToDay)   fsm_->dispatch(NightModeOff{}, now_ms);
}

void TrafficLightCoordinator::pollLightSensor(uint32_t now_ms) {
    const auto ev = ldr_->poll(now_ms);
    if (activeSource_ != NightSource::LightSensor) return;
    if (ev == LightSensor::Event::WentNight) fsm_->dispatch(NightModeOn{},  now_ms);
    if (ev == LightSensor::Event::WentDay)   fsm_->dispatch(NightModeOff{}, now_ms);
}

void TrafficLightCoordinator::toggleNight(uint32_t now_ms) {
    if (fsm_->in<NightBlink>()) {
        fsm_->dispatch(NightModeOff{}, now_ms);
    } else {
        fsm_->dispatch(NightModeOn{}, now_ms);
    }
}

void TrafficLightCoordinator::cycleSource(uint32_t now_ms) {
    switch (activeSource_) {
        case NightSource::Timer:       activeSource_ = NightSource::Button;      break;
        case NightSource::Button:      activeSource_ = NightSource::LightSensor; break;
        case NightSource::LightSensor: activeSource_ = NightSource::Timer;       break;
    }
    logSource();
    forceSyncToSource(now_ms);
}

void TrafficLightCoordinator::forceSyncToSource(uint32_t now_ms) {
    const bool fsmInNight = fsm_->in<NightBlink>();
    switch (activeSource_) {
        case NightSource::Timer: {
            const bool wantNight = !phase_->isDay();
            if (wantNight && !fsmInNight) fsm_->dispatch(NightModeOn{},  now_ms);
            if (!wantNight && fsmInNight) fsm_->dispatch(NightModeOff{}, now_ms);
            break;
        }
        case NightSource::LightSensor: {
            const auto r = ldr_->current();
            if (r == LightSensor::Reading::Night && !fsmInNight) fsm_->dispatch(NightModeOn{},  now_ms);
            if (r == LightSensor::Reading::Day   &&  fsmInNight) fsm_->dispatch(NightModeOff{}, now_ms);
            break;
        }
        case NightSource::Button:
            // No "current intent" — button is a toggle; leave FSM where it is.
            break;
    }
}

void TrafficLightCoordinator::logSource() const {
    Serial.printf("[coord] night source -> %s\n", sourceName(activeSource_));
}

const char* TrafficLightCoordinator::sourceName(NightSource s) {
    switch (s) {
        case NightSource::Timer:       return "timer";
        case NightSource::Button:      return "button";
        case NightSource::LightSensor: return "lightSensor";
    }
    return "?";
}

} // namespace pflash
