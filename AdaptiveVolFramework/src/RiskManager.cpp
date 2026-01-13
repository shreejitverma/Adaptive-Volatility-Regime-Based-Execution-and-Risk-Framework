#include "../include/adaptive_exec/RiskManager.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace AdaptiveExec {

    Scalar RiskManager::computeCVaR(const std::vector<Scalar>& returns, Scalar alpha) {
        if (returns.empty()) return 0.0;

        std::vector<Scalar> sorted_rets = returns;
        std::sort(sorted_rets.begin(), sorted_rets.end());

        // Index for alpha percentile
        size_t cutoff_idx = static_cast<size_t>(std::ceil(alpha * sorted_rets.size()));
        if (cutoff_idx == 0) cutoff_idx = 1;

        // CVaR = -Mean of returns below cutoff
        Scalar sum_tail = 0.0;
        for (size_t i = 0; i < cutoff_idx; ++i) {
            sum_tail += sorted_rets[i];
        }

        return -(sum_tail / cutoff_idx);
    }

    Scalar RiskManager::getRegimePositionSize(MarketRegime state, Scalar base_size) {
        Scalar multiplier = 1.0;
        switch (state) {
            case MarketRegime::LowVolatility:
                multiplier = 1.5;
                break;
            case MarketRegime::Normal:
                multiplier = 1.0;
                break;
            case MarketRegime::HighVolatility:
                multiplier = 0.5;
                break;
        }
        return base_size * multiplier;
    }

}
