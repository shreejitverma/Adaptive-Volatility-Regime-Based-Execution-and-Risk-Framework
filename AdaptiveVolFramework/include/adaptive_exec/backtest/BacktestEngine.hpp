#pragma once

#include "../Types.hpp"
#include "../ExecutionEngine.hpp" // For costs
#include "../analytics/PerformanceMetrics.hpp"
#include <vector>

namespace AdaptiveExec {

    struct Trade {
        int day;
        Scalar price;
        Scalar quantity;
        Scalar cost;
        Scalar pnl; // Realized PnL (simplified)
    };

    class BacktestEngine {
    public:
        BacktestEngine(Scalar initial_capital = 100000.0);

        // Reset state
        void reset(Scalar initial_capital);

        // Execute an order at a specific price
        // Returns cost of transaction
        void executeOrder(int day, Scalar price, Scalar quantity, MarketRegime regime);

        // Update Mark-to-Market value at end of day
        void updateEndOfDay(Scalar close_price);

        // Get results
        Vector getEquityCurve() const;
        std::vector<Trade> getTrades() const;
        MetricsResult getPerformanceMetrics() const;

    private:
        Scalar initial_capital_;
        Scalar cash_;
        Scalar position_; // Number of units
        Vector equity_curve_; 
        std::vector<Trade> trades_;
    };

}
