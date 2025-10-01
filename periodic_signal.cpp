#include "periodic_signal.hpp"
#include <iostream>
#include <ostream>

using std::chrono::duration;

PeriodicSignal::PeriodicSignal(int rate_limit_hz, OperationMode mode)
    : period_duration(std::chrono::milliseconds(1000 / rate_limit_hz)),
      last_signal_time(std::chrono::steady_clock::now()), last_delta_time(0.0), mode(mode) {}

double PeriodicSignal::get_cycle_progress() const {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_on_signal = now - last_signal_time;
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(time_since_last_on_signal).count();
    auto period = std::chrono::duration_cast<std::chrono::duration<double>>(period_duration).count();

    double progress = elapsed / period;
    if (progress < 0.0)
        progress = 0.0;
    if (progress > 1.0)
        progress = 1.0; // clamp just in case
    return progress;
}

bool PeriodicSignal::enough_time_has_passed() const {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_on_signal = now - last_signal_time;
    return time_since_last_on_signal >= period_duration;
}

bool PeriodicSignal::process_and_get_signal() {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_on_signal = now - last_signal_time;

    if (enough_time_has_passed()) {
        if (mode == OperationMode::PERFECT_DELTAS) {
            // NOTE: lying here vvv
            last_delta_time = std::chrono::duration_cast<std::chrono::duration<double>>(period_duration).count();

            // NOTE: if you move the signal time back in time, it means the next one comes up sooner, so subtracting the
            // left over time you had makes sense here
            auto leftover_time =
                time_since_last_on_signal % period_duration; // what if you were more than period duration late?
            last_signal_time = now - leftover_time;
            // NOTE: above we're pretending that the signal actually occurred at the time in the past when in reality it
            // didn't.
        } else {
            last_delta_time =
                std::chrono::duration_cast<std::chrono::duration<double>>(time_since_last_on_signal).count();
            last_signal_time = now;
        }
        return true;
    } else {
        return false;
    }
}

double PeriodicSignal::get_last_delta_time() const { return last_delta_time; }
