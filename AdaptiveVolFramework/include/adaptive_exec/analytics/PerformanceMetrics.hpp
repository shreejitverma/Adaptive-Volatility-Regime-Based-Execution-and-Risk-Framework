#pragma once

#include "../Types.hpp"
#include <vector>

namespace AdaptiveExec {

    struct MetricsResult {
        Scalar total_return;
        Scalar cagr;
        Scalar annualized_vol;
        Scalar sharpe_ratio;
        Scalar sortino_ratio;
        Scalar max_drawdown;
        Scalar win_rate;
    };

    class PerformanceMetrics {
    public:
        // Calculate metrics from a series of Equity Curve values (NAV)
        // Assume daily data (252 days/year)
        static MetricsResult calculate(const Vector& equity_curve);

    private:
        static Scalar calculateMaxDrawdown(const Vector& equity_curve);
    };

}
