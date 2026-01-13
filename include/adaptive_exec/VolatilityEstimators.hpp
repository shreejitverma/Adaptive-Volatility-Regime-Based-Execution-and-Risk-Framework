#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    class VolatilityEstimators {
    public:
        // Compute Realized Variance (RV)
        static Scalar computeRV(const std::vector<Scalar>& returns, Scalar annualization_factor = 252.0);

        // Compute Bipower Variation (BV) - Robust to jumps
        static Scalar computeBV(const std::vector<Scalar>& returns, Scalar annualization_factor = 252.0);

        // Compute Realized Jumps (RJ) = max(0, RV - BV)
        static Scalar computeRJ(const std::vector<Scalar>& returns, Scalar annualization_factor = 252.0);

        // Compute Two-Scale Realized Volatility (TSRV) - Robust to noise
        static Scalar computeTSRV(const std::vector<Scalar>& returns, int K = 5, Scalar annualization_factor = 252.0);

    private:
        // Helper to compute sum of squares
        static Scalar sumSquares(const std::vector<Scalar>& data);
    };

}
