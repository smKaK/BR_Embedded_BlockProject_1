#pragma once

#include "Flasher.h"
#include "IPattern.h"
#include "inputs/IController.h"

#include <initializer_list>

namespace pflash {

// Owns the registry of selectable patterns and the list of input controllers,
// polls the controllers each tick, and turns their events into Flasher state
// changes. Keeps main.cpp free of loose globals and free-function dispatch.
class ModeManager {
public:
    static constexpr uint8_t kMaxPatterns    = 8;
    static constexpr uint8_t kMaxControllers = 4;

    struct Config {
        Flasher*                            flasher;
        std::initializer_list<IPattern*>    patterns;
        std::initializer_list<IController*> controllers;
        uint8_t                             defaultIndex = 0;
    };

    explicit ModeManager(const Config& cfg);

    // Applies the default pattern and calls begin() on every controller.
    // The caller is expected to have already called flasher.begin().
    void begin();

    // Call every loop(): polls controllers, dispatches events.
    void tick();

    // Public actions (also reachable indirectly via ControlEvents).
    void cycleNext(const char* source);
    void applyPattern(uint8_t idx, const char* source);
    void printCurrent() const;
    void printHelp() const;

    uint8_t currentIndex() const { return idx_; }
    uint8_t patternCount() const { return patternCount_; }

private:
    void dispatch(const ControlEvent& ev, const char* source);

    Flasher*     flasher_;
    IPattern*    patterns_   [kMaxPatterns]   {};
    IController* controllers_[kMaxControllers]{};
    uint8_t      patternCount_   {0};
    uint8_t      controllerCount_{0};
    uint8_t      idx_            {0};
};

} // namespace pflash
