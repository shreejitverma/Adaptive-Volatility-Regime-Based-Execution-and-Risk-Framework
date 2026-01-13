#include "../include/adaptive_exec/ExecutionEngine.hpp"
#include <cmath>
#include <numeric>
#include <iostream>

namespace AdaptiveExec {

    Scalar ExecutionEngine::computeTransactionCosts(MarketRegime state, Scalar order_size, Scalar spread_bps) {
        Scalar spread_mult = 1.0;
        Scalar impact_coef = 0.05;

        switch (state) {
            case MarketRegime::LowVolatility:
                spread_mult = 0.8;
                impact_coef = 0.02;
                break;
            case MarketRegime::Normal:
                spread_mult = 1.0;
                impact_coef = 0.05;
                break;
            case MarketRegime::HighVolatility:
                spread_mult = 2.5;
                impact_coef = 0.15;
                break;
        }

        Scalar spread_cost = spread_bps * spread_mult;
        // Impact cost ~ sqrt(order_size)
        // Coef * sqrt(size)
        Scalar impact_cost = impact_coef * std::sqrt(order_size); 
        
        return spread_cost + impact_cost;
    }

    Vector ExecutionEngine::getExecutionSchedule(MarketRegime state, Scalar order_size, int time_horizon) {
        Vector schedule(time_horizon);

        if (state == MarketRegime::LowVolatility) {
            // Aggressive VWAP (Front-loaded)
            Scalar decay_rate = 0.3;
            for (int t = 0; t < time_horizon; ++t) {
                schedule[t] = order_size * decay_rate * std::exp(-decay_rate * t);
            }
        } else if (state == MarketRegime::Normal) {
            // TWAP (Uniform)
            schedule.fill(order_size / time_horizon);
        } else {
            // Passive (Back-loaded)
            Scalar decay_rate = 0.15;
            Vector weights(time_horizon);
            Scalar w_sum = 0.0;
            for (int t = 0; t < time_horizon; ++t) {
                weights[t] = std::exp(-decay_rate * t);
                w_sum += weights[t];
            }
            schedule = (weights / w_sum) * order_size;
        }

        // Normalize to ensure sum equals order_size
        Scalar current_sum = schedule.sum();
        if (current_sum > 0) {
            schedule *= (order_size / current_sum);
        }

        return schedule;
    }

    bool ExecutionEngine::checkCircuitBreaker(double current_intensity, double limit) {
        if (current_intensity > limit) {
            // In a real system, this would trigger an atomic shutdown flag
            return true;
        }
        return false;
    }

}