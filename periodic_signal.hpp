#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <chrono>

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
     * @brief returns true if a signal should have occurred since the last one and updates internal state
     */
    bool process_and_get_signal() {
        auto now = std::chrono::steady_clock::now();
        auto next_expected_signal_time = start_time + (++signal_count) * period_duration;

        if (now >= next_expected_signal_time) {
            // compute delta based on actual elapsed time
            last_delta_time = std::chrono::duration<double>(now - last_signal_time).count();
            last_signal_time = now;
            return true;
        } else {
            // revert increment if not yet time
            --signal_count;
            return false;
        }
    }

    /**
     * @brief returns the amount of time it took for the last signal to come through
     */
    double get_last_delta_time() const { return last_delta_time; }

    /**
     * @brief returns true if a signal should have occurred since the last signal.
     */
    bool enough_time_has_passed() const {
        auto now = std::chrono::steady_clock::now();
        auto next_expected_signal_time = start_time + (signal_count + 1) * period_duration;
        return now >= next_expected_signal_time;
    }

    /**
     * @brief returns progress (0.0–1.0) toward next expected signal
     */
    double get_cycle_progress() const {
        auto now = std::chrono::steady_clock::now();
        auto current_cycle_start = start_time + signal_count * period_duration;
        auto elapsed_in_cycle = std::chrono::duration<double>(now - current_cycle_start).count();
        double period_seconds = period_duration.count();
        return std::clamp(elapsed_in_cycle / period_seconds, 0.0, 1.0);
    }

  private:
    std::chrono::duration<double> period_duration;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_signal_time;
    int signal_count;
    double last_delta_time;
};

#endif // PERIODIC_SIGNAL_HPP
