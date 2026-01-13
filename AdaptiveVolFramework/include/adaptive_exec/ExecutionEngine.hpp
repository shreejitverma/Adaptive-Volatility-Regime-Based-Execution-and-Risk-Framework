#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    class ExecutionEngine {
    public:
        // Compute transaction cost in basis points
        static Scalar computeTransactionCosts(MarketRegime state, Scalar order_size, Scalar spread_bps = 5.0);

        // Generate execution schedule (VWAP/TWAP/Passive)
        // Returns vector of trade sizes for each period in horizon
        static Vector getExecutionSchedule(MarketRegime state, Scalar order_size, int time_horizon = 10);

        // HFT Safety: Check if trading should halt due to Hawkes intensity
        static bool checkCircuitBreaker(double current_intensity, double limit);
    };

}
