#include "../include/adaptive_exec/HawkesModel.hpp"
#include <cmath>
#include <algorithm>

namespace AdaptiveExec {

    HawkesModel::HawkesModel(double baseline, double alpha, double beta)
        : mu_(baseline), alpha_(alpha), beta_(beta), 
          last_event_time_(0.0), last_intensity_(baseline) {}

    double HawkesModel::addEvent(double timestamp) {
        // Recursive formula for Hawkes intensity:
        // lambda(t_now) = mu + (lambda(t_prev) - mu) * exp(-beta * (t_now - t_prev)) + alpha
        
        double dt = timestamp - last_event_time_;
        if (dt < 0) dt = 0; // Protection against out-of-order

        // Intensity just before this new event (decayed from last)
        double intensity_before = mu_ + (last_intensity_ - mu_) * std::exp(-beta_ * dt);
        
        // Intensity just after this new event (jump by alpha)
        double intensity_after = intensity_before + alpha_;

        // Update state
        last_event_time_ = timestamp;
        last_intensity_ = intensity_after;

        return intensity_after;
    }

    double HawkesModel::getIntensity(double timestamp) const {
        double dt = timestamp - last_event_time_;
        if (dt < 0) return last_intensity_; // Should not happen if time moves forward

        // Decay the excitement
        return mu_ + (last_intensity_ - mu_) * std::exp(-beta_ * dt);
    }

    bool HawkesModel::isCritical(double timestamp, double threshold) const {
        return getIntensity(timestamp) > threshold;
    }

    void HawkesModel::reset() {
        last_event_time_ = 0.0;
        last_intensity_ = mu_;
    }

}
