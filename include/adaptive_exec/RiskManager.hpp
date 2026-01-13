#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    class RiskManager {
    public:
        // Compute CVaR at 95% confidence (alpha = 0.05)
        static Scalar computeCVaR(const std::vector<Scalar>& returns, Scalar alpha = 0.05);

        // Get position size multiplier based on regime
        static Scalar getRegimePositionSize(MarketRegime state, Scalar base_size = 1.0);
    };

}
