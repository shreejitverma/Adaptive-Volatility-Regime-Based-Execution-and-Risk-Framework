#pragma once

#include "Types.hpp"
#include <vector>
#include <deque>

namespace AdaptiveExec {

    /**
     * @class HawkesModel
     * @brief Implements a univariate Self-Exciting Point Process (Hawkes Process).
     * 
     * Used for modeling the arrival intensity of trade events in high-frequency data.
     * The intensity \f$ \lambda(t) \f$ is defined as:
     * \f[
     * \lambda(t) = \mu + \sum_{t_i < t} \alpha e^{-\beta (t - t_i)}
     * \f]
     * 
     * This model captures "clustering" of events (volatility clustering, flash crashes).
     * The implementation uses a recursive formula for O(1) updates.
     */
    class HawkesModel {
    public:
        /**
         * @brief Construct a new Hawkes Model
         * 
         * @param baseline (mu) Background intensity rate (events per time unit)
         * @param alpha Excitation magnitude (jump in intensity per event)
         * @param beta Decay rate (speed at which excitement fades)
         */
        HawkesModel(double baseline, double alpha, double beta);

        /**
         * @brief Update the model with a new event occurrence.
         * 
         * @param timestamp Time of the new event (must be >= last event time)
         * @return double The intensity *immediately after* the event (including the jump)
         */
        double addEvent(double timestamp);

        /**
         * @brief Get the intensity at a specific query time.
         * 
         * @param timestamp Query time (must be >= last event time)
         * @return double Current intensity (decayed from last event)
         */
        double getIntensity(double timestamp) const;

        /**
         * @brief Check if intensity exceeds a critical safety threshold.
         * 
         * @param timestamp Current time
         * @param threshold Critical intensity limit
         * @return true If circuit breaker should trigger
         * @return false If safe
         */
        bool isCritical(double timestamp, double threshold) const;

        /**
         * @brief Reset model state (e.g., for start of new trading day).
         */
        void reset();

    private:
        double mu_;
        double alpha_;
        double beta_;
        
        double last_event_time_;
        double last_intensity_; // Intensity just after the last event
    };

}
