#ifndef PERIODIC_SIGNAL_HPP
#define PERIODIC_SIGNAL_HPP

#include <algorithm>
#include <chrono>
#include <cmath>

/**
 * @brief the operation mode indicates how the delta times are computed in the PeriodicSignal
 *
 * with perfect detlas, when enough time has elapsed for a delta time to have occurred, then it's reported that the last
 * delta time is given by 1/freq, ie the exact period, and not the actual measured time, this is good for when you want
 * to be able to reliably use the same delta time over and over for stability in certain systems, one example is in
 * client prediction and server reconciliation to guarentee that both systems use the same delta time to keep their
 * results similar
 *
 * measured deltas are exactly what they sound like and report how much time has actually passed since the last time the
 * periodic signal was checked and was ready to emit the signal
 *
 */
enum class DeltaMode {
    perfect,
    measured,
};

/**
 * @brief A class for generating periodic signals based on a specified rate with different operation modes
 *
 * @details The current implementation was found to be more accurate then the previous implementation, which can be
 * viewed by going back in the git history to when this comment was added, and viewing the periodic signals
 * implementation, in the old implementation everything was based off the last time and it would have  error building up
 * over time, with the new implemenation everything is thought of as a timeline, and the "ticks" are laid out before
 * hand, we just sample the clock and figure out which tick we are in, which has shown to be more accurate.
 */

class PeriodicSignal {

  public:
    /**
     * @brief Specifies how time is perceived by the PeriodicSignal.
     *
     * @details
     * This enum controls the "time model" used for computing signal deltas and
     * cycle progress:
     *
     * - realtime: Uses the actual measured time since the last signal. Each call
     *   to PeriodicSignal functions observes the current real clock time.
     *
     * - tick_latched: Time is latched per tick. All calls within the same tick
     *   see the same timestamp and delta time, ensuring deterministic behavior
     *   across subsystems. Useful for simulations, client prediction, or server
     *   reconciliation.
     *
     * @warn tick_latched behavior is not yet implemented
     */
    enum class TimeModel {
        realtime,
        tick_latched,
    };

    explicit PeriodicSignal(int rate_limit_hz, DeltaMode delta_mode = DeltaMode::measured,
                            TimeModel time_model = TimeModel::realtime)
        : period_duration(std::chrono::duration<double>(1.0 / rate_limit_hz)),
          start_time(std::chrono::steady_clock::now()), signal_count(0), last_signal_time(start_time),
          last_delta_time(0.0), delta_mode(delta_mode) {}

    /**
     * @brief Returns true if one or more signals should have occurred since the last call.
     *        If we have fallen behind, it "catches up" to the latest expected signal.
     * @note this is the function that should be called when you want to do something with the signal
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

    /**
     * @brief returns the amount of time it took for the last signal to come through
     *
     * when the operation mode is PERFECT_DELTAS, this function lies and always returns the computed period based off
     * rate_limit_hz when the operation mode is MEASURED_DELTAS, this function returns the measured amount of time
     *
     */
    double get_last_delta_time() const {
        if (delta_mode == DeltaMode::perfect) {
            return period_duration.count();
        }
        return last_delta_time;
    }

    /**
     * @brief returns true if a signal would have occurred since the last signal.
     */
    bool enough_time_has_passed() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        int expected_signal_count = static_cast<int>(elapsed_seconds / period_duration.count());
        return expected_signal_count > signal_count;
    }

    /**
     * @brief Returns normalized progress [0,1] through the current cycle.
     */
    double get_cycle_progress() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        double cycle_position = std::fmod(elapsed_seconds, period_duration.count());
        return std::clamp(cycle_position / period_duration.count(), 0.0, 1.0);
    }

  private:
    DeltaMode delta_mode;
    TimeModel time_model;
    std::chrono::duration<double> period_duration;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_signal_time;
    int signal_count;
    double last_delta_time;
};

#endif // PERIODIC_SIGNAL_HPP
