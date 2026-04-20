// fsm.hpp — header-only, template-based finite state machine for embedded C++.
//
// Design goals:
//   * Header-only, C++17. No heap, no RTTI, no exceptions, no virtuals.
//   * Type-safe states and events via std::variant + overload resolution.
//   * CRTP: the derived class owns the (state, event) handler overloads;
//     the base provides dispatch, entry/exit plumbing, and timeouts.
//   * O(1) dispatch: std::visit lowers to a jump table; the event type is
//     known at compile time, so every call site inlines to a direct branch.
//
// Minimum viable user class:
//
//     struct Idle {};
//     struct Running { int cycles = 0; };
//     struct Tick {};
//
//     class MyFsm : public fsm::StateMachine<MyFsm, Idle, Running> {
//     public:
//         Trans handle(Idle&,    const Tick&) { return Trans::to<Running>(); }
//         Trans handle(Running&, const Tick&) { return Trans::stay();        }
//         template <class S, class E> Trans handle(S&, const E&) { return Trans::stay(); }
//
//         void on_enter(Running&) { set_timeout(1000); }
//         Trans handle(Running& r, const fsm::Timeout&) {
//             ++r.cycles; set_timeout(1000); return Trans::stay();
//         }
//     };

#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>

namespace fsm {

// Synthetic event delivered by tick() when a state's set_timeout() elapses.
struct Timeout {};

// Transition returned from a handle() overload. Either "stay" or a concrete
// next-state value. Stored as variant<monostate, States...>; monostate = stay.
template <typename... States>
class Transition {
public:
    using Variant = std::variant<std::monostate, States...>;

    Transition() = default;

    static Transition stay() { return Transition{}; }

    template <class S>
    static Transition to() {
        Transition t;
        t.next_.template emplace<S>();
        return t;
    }

    template <class S>
    static Transition to(S&& s) {
        Transition t;
        t.next_.template emplace<std::decay_t<S>>(std::forward<S>(s));
        return t;
    }

    bool is_stay() const {
        return std::holds_alternative<std::monostate>(next_);
    }

    Variant& value() { return next_; }

private:
    Variant next_{};
};

namespace detail {

// Detection idiom: does Derived define on_enter(State&) / on_exit(State&)?
template <typename, typename = void>
struct has_on_enter : std::false_type {};
template <typename T>
struct has_on_enter<T, std::void_t<decltype(std::declval<T&>().on_enter(
                           std::declval<typename T::DetectState&>()))>>
    : std::true_type {};

template <typename, typename = void>
struct has_on_exit : std::false_type {};
template <typename T>
struct has_on_exit<T, std::void_t<decltype(std::declval<T&>().on_exit(
                          std::declval<typename T::DetectState&>()))>>
    : std::true_type {};

// Generic per-state detection — the struct above works on DetectState alias
// tricks; here we use a direct SFINAE probe keyed on the actual state type.
template <typename D, typename S, typename = void>
struct derived_has_on_enter : std::false_type {};
template <typename D, typename S>
struct derived_has_on_enter<
    D, S,
    std::void_t<decltype(std::declval<D&>().on_enter(std::declval<S&>()))>>
    : std::true_type {};

template <typename D, typename S, typename = void>
struct derived_has_on_exit : std::false_type {};
template <typename D, typename S>
struct derived_has_on_exit<
    D, S,
    std::void_t<decltype(std::declval<D&>().on_exit(std::declval<S&>()))>>
    : std::true_type {};

}  // namespace detail

// CRTP base. `Derived` supplies handle() overloads and optional on_enter /
// on_exit overloads. States... must be complete, move-constructible types.
template <typename Derived, typename... States>
class StateMachine {
public:
    using StateVariant = std::variant<States...>;
    using Trans        = Transition<States...>;

    // Initialize the machine with `initial` as the current state. Runs the
    // entry hook for that state (if defined). `now_ms` is whatever monotonic
    // millisecond clock the caller uses (millis() on Arduino, etc.).
    template <class Initial>
    void begin(Initial&& initial, uint32_t now_ms) {
        state_.template emplace<std::decay_t<Initial>>(
            std::forward<Initial>(initial));
        state_entered_ms_ = now_ms;
        timeout_ms_       = 0;
        invoke_entry();
    }

    // Synchronously route `ev` to the handler for the current state. If the
    // handler returns a non-stay transition, the old state's on_exit runs,
    // the new state is installed, its timeout is reset, and on_enter runs —
    // all before dispatch() returns.
    template <class Event>
    void dispatch(const Event& ev, uint32_t now_ms) {
        // Not re-entrant: calling dispatch() from within a handler would
        // mutate state_ while visit() is reading it.
        if (in_dispatch_) return;
        in_dispatch_ = true;

        Trans result = std::visit(
            [&](auto& s) -> Trans {
                Derived& self = *static_cast<Derived*>(this);
                return self.handle(s, ev);
            },
            state_);

        apply_transition(std::move(result), now_ms);
        in_dispatch_ = false;
    }

    // Poll hook. If a timeout is armed and has expired, synthesize a Timeout
    // event and dispatch it. Safe to call every loop() iteration.
    void tick(uint32_t now_ms) {
        if (timeout_ms_ != 0 &&
            (uint32_t)(now_ms - state_entered_ms_) >= timeout_ms_) {
            // Disarm before dispatch so a handler that doesn't re-arm via
            // set_timeout() won't refire on the next tick.
            timeout_ms_ = 0;
            dispatch(Timeout{}, now_ms);
        }
    }

    // Arm (or disarm, with 0) a timeout for the *current* state. Cleared on
    // every transition, so each state is responsible for re-arming from its
    // on_enter() or from its Timeout handler.
    void set_timeout(uint32_t ms) { timeout_ms_ = ms; }

    template <class S>
    bool in() const {
        return std::holds_alternative<S>(state_);
    }

    const StateVariant& state() const { return state_; }

private:
    void apply_transition(Trans&& t, uint32_t now_ms) {
        if (t.is_stay()) return;

        invoke_exit();

        // Move the new state out of the transition's variant into state_.
        std::visit(
            [&](auto&& ns) {
                using T = std::decay_t<decltype(ns)>;
                if constexpr (!std::is_same_v<T, std::monostate>) {
                    state_.template emplace<T>(std::move(ns));
                }
            },
            t.value());

        state_entered_ms_ = now_ms;
        timeout_ms_       = 0;
        invoke_entry();
    }

    void invoke_entry() {
        std::visit(
            [this](auto& s) {
                using S       = std::decay_t<decltype(s)>;
                Derived& self = *static_cast<Derived*>(this);
                if constexpr (detail::derived_has_on_enter<Derived, S>::value) {
                    self.on_enter(s);
                }
            },
            state_);
    }

    void invoke_exit() {
        std::visit(
            [this](auto& s) {
                using S       = std::decay_t<decltype(s)>;
                Derived& self = *static_cast<Derived*>(this);
                if constexpr (detail::derived_has_on_exit<Derived, S>::value) {
                    self.on_exit(s);
                }
            },
            state_);
    }

    StateVariant state_{};
    uint32_t     state_entered_ms_ = 0;
    uint32_t     timeout_ms_       = 0;
    bool         in_dispatch_      = false;
};

}  // namespace fsm

/*
Example

// States are plain value types — std::variant copies/moves them by value, so
// any per-state data (like led_on) lives here rather than in the FSM class.
struct Off       {};
struct SlowBlink { bool led_on = false; };
struct FastBlink { bool led_on = false; };

struct ButtonPress {};

class BlinkFsm : public fsm::StateMachine<BlinkFsm, Off, SlowBlink, FastBlink> {
public:
    // --- Button press cycles through the three modes. ---
    Trans handle(Off&,       const ButtonPress&) { return Trans::to<SlowBlink>(); }
    Trans handle(SlowBlink&, const ButtonPress&) { return Trans::to<FastBlink>(); }
    Trans handle(FastBlink&, const ButtonPress&) { return Trans::to<Off>();       }

    // --- Timeout is how each blink state advances its own LED. ---
    Trans handle(SlowBlink& s, const fsm::Timeout&) {
        s.led_on = !s.led_on;
        digitalWrite(kLedPin, s.led_on ? HIGH : LOW);
        set_timeout(kSlowPeriodMs);
        return Trans::stay();
    }
    Trans handle(FastBlink& s, const fsm::Timeout&) {
        s.led_on = !s.led_on;
        digitalWrite(kLedPin, s.led_on ? HIGH : LOW);
        set_timeout(kFastPeriodMs);
        return Trans::stay();
    }

    // Catch-all: every other (state, event) combination stays put.
    template <class S, class E>
    Trans handle(S&, const E&) { return Trans::stay(); }

    // --- Entry actions: establish the LED state and arm timeouts. ---
    void on_enter(Off&)         { digitalWrite(kLedPin, LOW); }
    void on_enter(SlowBlink& s) { s.led_on = true;  digitalWrite(kLedPin, HIGH); set_timeout(kSlowPeriodMs); }
    void on_enter(FastBlink& s) { s.led_on = true;  digitalWrite(kLedPin, HIGH); set_timeout(kFastPeriodMs); }

    // --- Exit actions: leave the LED in a known state between transitions. ---
    void on_exit(SlowBlink&) { digitalWrite(kLedPin, LOW); }
    void on_exit(FastBlink&) { digitalWrite(kLedPin, LOW); }
};

BlinkFsm g_fsm;

*/