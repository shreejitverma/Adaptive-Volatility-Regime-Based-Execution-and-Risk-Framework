#pragma once

#include <Eigen/Dense>
#include <vector>

namespace AdaptiveExec {

    // Use double precision for financial calculations
    using Scalar = double;
    using Vector = Eigen::VectorXd;
    using Matrix = Eigen::MatrixXd;
    using RowVector = Eigen::RowVectorXd;

    // Structure for daily market data
    struct MarketData {
        Scalar open;
        Scalar high;
        Scalar low;
        Scalar close;
        Scalar volume;
        std::vector<Scalar> intraday_prices; // For high-frequency calcs
    };

    // Enumeration for HMM Regimes
    enum class MarketRegime {
        LowVolatility = 0,
        Normal = 1,
        HighVolatility = 2
    };

}
