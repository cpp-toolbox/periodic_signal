#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <chrono>

/**
 * @brief A class for generating periodic signals based on a specified rate.
 */
class PeriodicSignal {
  public:
    explicit PeriodicSignal(int rate_limit_hz);
    bool process_and_get_signal();
    double get_last_delta_time() const;

  private:
    std::chrono::milliseconds period_duration;              ///< The time interval between signals.
    std::chrono::steady_clock::time_point last_signal_time; ///< The time when the last signal was emitted.
    double last_delta_time; ///< The time difference since the last processing in seconds.
};

#endif // PERIODIC_SIGNAL_HPP
