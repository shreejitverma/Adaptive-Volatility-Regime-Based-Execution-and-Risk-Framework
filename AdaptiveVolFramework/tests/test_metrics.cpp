#include <gtest/gtest.h>
#include "../include/adaptive_exec/analytics/PerformanceMetrics.hpp"

using namespace AdaptiveExec;

TEST(PerformanceMetricsTest, MaxDrawdownCalculation) {
    Vector equity(5);
    equity << 100, 110, 99, 105, 120; 
    // Peak at 110, Drop to 99. DD = (110-99)/110 = 11/110 = 0.10
    
    MetricsResult m = PerformanceMetrics::calculate(equity);
    EXPECT_NEAR(m.max_drawdown, 0.10, 1e-5);
}

TEST(PerformanceMetricsTest, SharpeRatioPositive) {
    Vector equity(4);
    equity << 100, 101, 102, 103; // Steady 1% gain
    
    MetricsResult m = PerformanceMetrics::calculate(equity);
    // Mean return approx 0.01 (daily)
    // Volatility very low
    // Sharpe should be high positive
    EXPECT_GT(m.sharpe_ratio, 1.0);
}
