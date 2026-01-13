#include "../include/adaptive_exec/analytics/PerformanceMetrics.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <iostream>

namespace AdaptiveExec {

    MetricsResult PerformanceMetrics::calculate(const Vector& equity_curve) {
        MetricsResult res = {0,0,0,0,0,0,0};
        long n = equity_curve.size();
        if (n < 2) return res;

        // 1. Calculate Returns
        Vector returns(n - 1);
        for (long i = 0; i < n - 1; ++i) {
            returns[i] = (equity_curve[i+1] - equity_curve[i]) / equity_curve[i];
        }

        // 2. Total Return & CAGR
        Scalar start_val = equity_curve[0];
        Scalar end_val = equity_curve[n - 1];
        res.total_return = (end_val - start_val) / start_val;
        
        Scalar years = (Scalar)n / 252.0;
        if (years > 0 && start_val > 0 && end_val > 0) {
            res.cagr = std::pow(end_val / start_val, 1.0 / years) - 1.0;
        }

        // 3. Volatility (Annualized)
        Scalar mean_ret = returns.mean();
        Scalar sq_sum = (returns.array() - mean_ret).square().sum();
        Scalar std_dev = std::sqrt(sq_sum / (n - 2)); // Sample std dev
        res.annualized_vol = std_dev * std::sqrt(252.0);

        // 4. Sharpe Ratio (Risk Free Rate = 0 for simplicity)
        if (res.annualized_vol > 0) {
            res.sharpe_ratio = (mean_ret * 252.0) / res.annualized_vol;
        }

        // 5. Sortino Ratio
        // Only negative returns contribute to downside deviation
        Vector neg_rets = (returns.array() < 0).select(returns, 0.0);
        Scalar downside_sq_sum = neg_rets.squaredNorm();
        Scalar downside_dev = std::sqrt(downside_sq_sum / (n - 2));
        Scalar ann_downside_vol = downside_dev * std::sqrt(252.0);
        
        if (ann_downside_vol > 0) {
            res.sortino_ratio = (mean_ret * 252.0) / ann_downside_vol;
        }

        // 6. Max Drawdown
        res.max_drawdown = calculateMaxDrawdown(equity_curve);

        // 7. Win Rate
        long wins = (returns.array() > 0).count();
        res.win_rate = (Scalar)wins / (Scalar)returns.size();

        return res;
    }

    Scalar PerformanceMetrics::calculateMaxDrawdown(const Vector& equity_curve) {
        Scalar max_dd = 0.0;
        Scalar peak = equity_curve[0];

        for (long i = 0; i < equity_curve.size(); ++i) {
            if (equity_curve[i] > peak) {
                peak = equity_curve[i];
            }
            Scalar dd = (peak - equity_curve[i]) / peak;
            if (dd > max_dd) {
                max_dd = dd;
            }
        }
        return max_dd;
    }

}
