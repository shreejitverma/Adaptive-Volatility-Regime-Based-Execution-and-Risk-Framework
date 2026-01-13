#include "../include/adaptive_exec/VolatilityEstimators.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>

namespace AdaptiveExec {

    Scalar VolatilityEstimators::sumSquares(const std::vector<Scalar>& data) {
        Scalar sum = 0.0;
        for (Scalar val : data) {
            sum += val * val;
        }
        return sum;
    }

    Scalar VolatilityEstimators::computeRV(const std::vector<Scalar>& returns, Scalar annualization_factor) {
        Scalar rv = sumSquares(returns);
        return rv * annualization_factor;
    }

    Scalar VolatilityEstimators::computeBV(const std::vector<Scalar>& returns, Scalar annualization_factor) {
        if (returns.size() < 2) return 0.0;
        
        Scalar sum_abs_prod = 0.0;
        for (size_t i = 0; i < returns.size() - 1; ++i) {
            sum_abs_prod += std::abs(returns[i]) * std::abs(returns[i+1]);
        }
        
        Scalar bv = (M_PI / 2.0) * sum_abs_prod;
        return bv * annualization_factor;
    }

    Scalar VolatilityEstimators::computeMedRV(const std::vector<Scalar>& returns, Scalar annualization_factor) {
        if (returns.size() < 3) return 0.0;

        Scalar sum_med_sq = 0.0;
        Scalar scale_factor = (M_PI) / (6.0 - 4.0 * std::sqrt(3.0) + M_PI); // Approx 1.419
        Scalar correction = static_cast<Scalar>(returns.size()) / static_cast<Scalar>(returns.size() - 2);

        for (size_t i = 2; i < returns.size(); ++i) {
            Scalar a = std::abs(returns[i]);
            Scalar b = std::abs(returns[i-1]);
            Scalar c = std::abs(returns[i-2]);
            
            // Median of 3
            Scalar med;
            if ((a <= b && b <= c) || (c <= b && b <= a)) med = b;
            else if ((b <= a && a <= c) || (c <= a && a <= b)) med = a;
            else med = c;

            sum_med_sq += med * med;
        }

        return scale_factor * correction * sum_med_sq * annualization_factor;
    }

    std::vector<Scalar> VolatilityEstimators::computeLeeMykland(const std::vector<Scalar>& returns, size_t window_size) {
        std::vector<Scalar> statistics(returns.size(), 0.0);
        if (returns.size() <= window_size + 1) return statistics;

        // Pre-compute constant for local BV
        Scalar c_bv = M_PI / 2.0;

        for (size_t i = window_size; i < returns.size(); ++i) {
            // Compute Local Volatility using BV over [i - window_size, i - 1]
            Scalar local_bv_sum = 0.0;
            // Iterate window. If window_size is K, we need K-1 products for BV
            // Window of returns: r[i-K], ..., r[i-1]
            // We need pairs (r[j], r[j+1])
            
            size_t start_idx = i - window_size;
            size_t valid_pairs = 0;

            for (size_t j = start_idx; j < i - 1; ++j) {
                local_bv_sum += std::abs(returns[j]) * std::abs(returns[j+1]);
                valid_pairs++;
            }

            if (valid_pairs == 0) continue;

            Scalar local_variance = c_bv * (local_bv_sum / static_cast<Scalar>(valid_pairs));
            Scalar local_vol = std::sqrt(local_variance);

            if (local_vol > 1e-9) {
                statistics[i] = std::abs(returns[i]) / local_vol;
            } else {
                statistics[i] = 0.0;
            }
        }
        return statistics;
    }

    Scalar VolatilityEstimators::computeRJ(const std::vector<Scalar>& returns, Scalar annualization_factor) {
        Scalar rv = computeRV(returns, annualization_factor);
        Scalar bv = computeBV(returns, annualization_factor);
        return std::max(0.0, rv - bv);
    }

    Scalar VolatilityEstimators::computeTSRV(const std::vector<Scalar>& returns, int K, Scalar annualization_factor) {
        size_t n = returns.size();
        if (n < (size_t)K) return 0.0;

        // RV all
        Scalar rv_all = sumSquares(returns);

        // RV subsampled (every Kth)
        Scalar rv_sub = 0.0;
        // The python code does: rv_sub = K * np.sum(returns[::K]**2)
        // This is an approximation. A more rigorous TSRV averages across K grids.
        // But following the Python notebook exactly:
        for (size_t i = 0; i < n; i += K) {
            rv_sub += returns[i] * returns[i];
        }
        rv_sub *= K;

        // TSRV formula
        // tsrv = rv_all - (rv_sub / n) * (n - K + 1) / K
        // Wait, Python notebook formula: tsrv = rv_all - (rv_sub / n) * (n - K + 1) / K
        // Let's implement that exactly.
        
        Scalar term2 = (rv_sub / static_cast<Scalar>(n)) * (static_cast<Scalar>(n - K + 1)) / static_cast<Scalar>(K);
        Scalar tsrv = rv_all - term2;

        return std::max(0.0, tsrv) * annualization_factor;
    }

}
