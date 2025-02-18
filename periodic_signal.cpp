#include "periodic_signal.hpp"

PeriodicSignal::PeriodicSignal(int rate_limit_hz)
    : period_duration(std::chrono::milliseconds(1000 / rate_limit_hz)),
      last_signal_time(std::chrono::steady_clock::now()), last_delta_time(0.0) {}

bool PeriodicSignal::process_and_get_signal() {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_on_signal = now - last_signal_time;

    // Update the last delta time in seconds
    last_delta_time = std::chrono::duration_cast<std::chrono::duration<double>>(time_since_last_on_signal).count();

    if (time_since_last_on_signal >= period_duration) {
        // Calculate the leftover time after the last full period
        auto leftover_time = time_since_last_on_signal % period_duration;

        // Adjust last_signal_time to "pay forward" the leftover time into the next computation
        // think of it as "moving the ticker back"
        last_signal_time = now - leftover_time;
        return true;
    } else {
        return false;
    }
}

double PeriodicSignal::get_last_delta_time() const { return last_delta_time; }
