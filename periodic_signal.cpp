#include "periodic_signal.hpp"
#include <iostream>
#include <ostream>

using std::chrono::duration;

PeriodicSignal::PeriodicSignal(int rate_limit_hz, OperationMode mode)
    : period_duration(std::chrono::milliseconds(1000 / rate_limit_hz)),
      last_signal_time(std::chrono::steady_clock::now()), last_delta_time(0.0), mode(mode) {}

bool PeriodicSignal::process_and_get_signal() {
    auto now = std::chrono::steady_clock::now();
    auto time_since_last_on_signal = now - last_signal_time;

    /*std::cout << "time since last on signal: "*/
    /*          << std::chrono::duration_cast<duration<double>>(time_since_last_on_signal).count() << std::endl;*/

    if (time_since_last_on_signal >= period_duration) {
        if (mode == OperationMode::PERFECT_DELTAS) {
            last_delta_time = std::chrono::duration_cast<std::chrono::duration<double>>(period_duration).count();
            auto leftover_time = time_since_last_on_signal % period_duration;
            last_signal_time = now - leftover_time;
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
