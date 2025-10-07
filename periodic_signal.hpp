#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <algorithm>
#include <chrono>
#include <cmath>

enum class OperationMode {
    PERFECT_DELTAS,
    MEASURED_DELTAS,
};

/**
 * @brief A class for generating periodic signals based on a specified rate with different operation modes
 */

class PeriodicSignal {
  public:
    explicit PeriodicSignal(int rate_limit_hz, OperationMode mode = OperationMode::MEASURED_DELTAS);

    /**
     * @breif returns true if a signal would have occurred since the last signal and updates internal state
     * @note this is the function that should be called when you want to do something with the signal
     */
    bool process_and_get_signal();
    /**
     * @brief returns the amount of time it took for the last signal to come through
     *
     * when the operation mode is PERFECT_DELTAS, this function lies and always returns the computed period based off
     * rate_limit_hz when the operation mode is MEASURED_DELTAS, this function returns the measured amount of time
     *
     */
    double get_last_delta_time() const;
    /**
     * @brief returns true if a signal would have occurred since the last signal.
     */
    bool enough_time_has_passed() const;
    /**
     * @brief tells us how close we are until our next signal
     */
    double get_cycle_progress() const;

  private:
    OperationMode mode;
    std::chrono::milliseconds period_duration;              ///< The time interval between signals.
    std::chrono::steady_clock::time_point last_signal_time; ///< The time when the last signal was emitted.
    double last_delta_time; ///< The time difference since the last processing in seconds.
};

/**
 * @brief an alternate periodic signal that operates on the timeline rather than the above logic.
 */

class PeriodicSignalNew {
  public:
    explicit PeriodicSignalNew(int rate_limit_hz)
        : period_duration(std::chrono::duration<double>(1.0 / rate_limit_hz)),
          start_time(std::chrono::steady_clock::now()), signal_count(0), last_signal_time(start_time),
          last_delta_time(0.0) {}

    /**
     * @brief Returns true if one or more signals should have occurred since the last call.
     *        If we have fallen behind, it "catches up" to the latest expected signal.
     */
    bool process_and_get_signal() {
        auto now = std::chrono::steady_clock::now();

        // Compute how many full periods have elapsed since start
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        int expected_signal_count = static_cast<int>(elapsed_seconds / period_duration.count());

        // If we've reached or passed at least one new signal since last time
        if (expected_signal_count > signal_count) {
            // Move signal count to the latest one
            signal_count = expected_signal_count;
            last_delta_time = std::chrono::duration<double>(now - last_signal_time).count();
            last_signal_time = now;
            return true;
        }
        return false;
    }

    double get_last_delta_time() const { return last_delta_time; }

    bool enough_time_has_passed() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        int expected_signal_count = static_cast<int>(elapsed_seconds / period_duration.count());
        return expected_signal_count > signal_count;
    }

    double get_cycle_progress() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        double cycle_position = std::fmod(elapsed_seconds, period_duration.count());
        return std::clamp(cycle_position / period_duration.count(), 0.0, 1.0);
    }

  private:
    std::chrono::duration<double> period_duration;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_signal_time;
    int signal_count;
    double last_delta_time;
};

#endif // PERIODIC_SIGNAL_HPP
