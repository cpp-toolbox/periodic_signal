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
 *
 * @todo the start time is upon creation maybe we can make that a dynamic choice in the futre
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

    double cycle_progress_at_last_process_and_get_signal_call = 0;

    /**
     * @brief Resets the periodic signal to its initial state.
     *
     * @details This function restarts the timing of the signal, setting the start time
     *          to the current time and resetting the signal count and last signal time.
     *          After calling this, the signal behaves as if it has just been created.
     *
     * @note This does not change the signal rate or operation mode.
     */
    void restart() {
        start_time = std::chrono::steady_clock::now();
        signal_count = 0;
        last_signal_time = start_time;
        last_delta_time = 0.0;
    }

    /**
     * @brief Returns true if one or more signals should have occurred since the last call.
     *        If we have fallen behind, it "catches up" to the latest expected signal.
     * @note this is the function that should be called when you want to do something with the signal
     */
    bool process_and_get_signal() {
        auto now = std::chrono::steady_clock::now();

        // Compute how many full periods have elapsed since start
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        // this is the floor function here.
        int expected_signal_count = static_cast<int>(elapsed_seconds / period_duration.count());

        cycle_progress_at_last_process_and_get_signal_call = get_cycle_progress_at(now);

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
     *
     * @warn you only know that a new cycle has commenced by using @see process_and_get_signal, thus if you're checking
     * this value without first knowing if a new signal has started then you'll go from a value around 1 to a value
     * around 0. This most likely will cause unexpected behavior if you're doing any type of interpolation or any logic
     * with this value because you'll use this value in the context of still not knowing that a new signal has already
     * happened
     *
     *
     */
    double get_cycle_progress() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        double cycle_position = std::fmod(elapsed_seconds, period_duration.count());
        return std::clamp(cycle_position / period_duration.count(), 0.0, 1.0);
    }

    /**
     * @brief Returns normalized progress [0,1] through the cycle at a given time point.
     *
     * @details This function behaves identically to @see get_cycle_progress(), but instead
     *          of sampling the current clock, it computes the progress at the supplied
     *          time point. This is useful when you want deterministic sampling or when
     *          the caller already has a timestamp.
     *
     * @param time_point The time at which to compute cycle progress.
     *
     * @return A double in the range [0,1] representing progress through the cycle.
     */
    double get_cycle_progress_at(std::chrono::steady_clock::time_point time_point) const {
        double elapsed_seconds = std::chrono::duration<double>(time_point - start_time).count();
        double cycle_position = std::fmod(elapsed_seconds, period_duration.count());
        return std::clamp(cycle_position / period_duration.count(), 0.0, 1.0);
    }

    /**
     * @brief Returns normalized progress [0,1] through the current cycle, clamped if behind schedule.
     *
     * @note This function computes the progress through the current cycle similar to @see get_cycle_progress() but
     * integrates with the internal book keeping about when a signal is emitted. Fully that means that if we have
     * theoretically entered a new cycle at the time that this is called, but we still have not updated that internal
     * state through the @see process_and_get_signal function then this doesn't return a value close to 0 (indicating
     * that a new cycle ocurred), but will instead clamp at 1.
     *
     * The benefit of this is that you can use this function without having to check first if a new cycle occurred and
     * there will be no suprises in terms of what you expected.
     *
     * @return A double in the range [0,1] representing the progress through the current cycle.
     *
     * @note This "clamping" behavior prevents the progress from appearing to go backward if
     * the the call to process_and_get_signal delayed. If you do not need this behavior, consider using
     * @see get_cycle_progress() instead.
     */
    double get_cycle_progress_clamped() const {
        auto now = std::chrono::steady_clock::now();
        double elapsed_seconds = std::chrono::duration<double>(now - start_time).count();
        int expected_signal_count = static_cast<int>(elapsed_seconds / period_duration.count());

        if (expected_signal_count > signal_count) {
            // We are behind, so we "max out" progress
            return 1.0;
        }

        // Otherwise, compute progress normally
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
