#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <chrono>

/**
 * @brief A class for generating periodic signals based on a specified rate.
 */
class PeriodicSignal {
public:
    /**
     * @brief Constructor to initialize the periodic signal generator.
     * @param rate_limit_hz The desired frequency of the signal in hertz.
     */
    explicit PeriodicSignal(int rate_limit_hz);

    /**
     * @brief Determines if enough time has passed to emit a new signal.
     * @return True if the signal should be emitted, false otherwise.
     */
    bool process_and_get_signal();

private:
    std::chrono::milliseconds period_duration;   ///< The time interval between signals.
    std::chrono::steady_clock::time_point last_signal_time; ///< The time when the last signal was emitted.
};

#endif // PERIODIC_SIGNAL_HPP
