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

#endif // PERIODIC_SIGNAL_HPP
