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
