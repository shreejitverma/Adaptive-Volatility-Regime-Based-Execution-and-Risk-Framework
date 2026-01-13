#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    class VolatilityEstimators {
    public:
        // Standard Realized Volatility (sum of squared returns)
        static Scalar computeRV(const std::vector<Scalar>& returns, Scalar annualization_factor = 1.0);

        // Bipower Variation (robust to jumps)
        static Scalar computeBV(const std::vector<Scalar>& returns, Scalar annualization_factor = 1.0);

        // Median Integrated Volatility (MedRV) - More robust to jumps/outliers than BV
        static Scalar computeMedRV(const std::vector<Scalar>& returns, Scalar annualization_factor = 1.0);

        // Realized Jumps (RV - BV), floored at 0
        static Scalar computeRJ(const std::vector<Scalar>& returns, Scalar annualization_factor = 1.0);

        // Two-Scale Realized Volatility (robust to microstructure noise)
        static Scalar computeTSRV(const std::vector<Scalar>& returns, int K = 5, Scalar annualization_factor = 1.0);

        // Lee-Mykland Jump Detection Test
        // Returns a vector of t-statistics for each return.
        // Statistic > threshold (approx 3.0-3.5) implies a jump.
        // window_size: Local window for instantaneous volatility estimation (e.g., 16 to 270)
        static std::vector<Scalar> computeLeeMykland(const std::vector<Scalar>& returns, size_t window_size = 16);

    private:
        static Scalar sumSquares(const std::vector<Scalar>& data);
    };

}
