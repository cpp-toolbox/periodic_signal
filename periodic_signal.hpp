#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <chrono>

/**
 * @brief A class for generating periodic signals based on a specified rate.
 */
class PeriodicSignal {
  public:
    /**
     * @brief Constructs a PeriodicSignal with the specified rate limit.
     * @param rate_limit_hz The rate limit in Hertz (signals per second).
     */
    explicit PeriodicSignal(int rate_limit_hz);

    /**
     * @brief Processes the signal and determines whether it is time to emit the next signal.
     * @return True if the signal is emitted, false otherwise.
     */
    bool process_and_get_signal();

    /**
     * @brief Gets the time difference since the last signal processing.
     * @return The time difference in seconds as a double.
     */
    double get_last_delta_time() const;

  private:
    std::chrono::milliseconds period_duration;              ///< The time interval between signals.
    std::chrono::steady_clock::time_point last_signal_time; ///< The time when the last signal was emitted.
    double last_delta_time; ///< The time difference since the last processing in seconds.
};

#endif // PERIODIC_SIGNAL_HPP
