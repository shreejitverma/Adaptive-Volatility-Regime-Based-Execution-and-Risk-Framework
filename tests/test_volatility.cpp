#include <gtest/gtest.h>
#include "../include/adaptive_exec/VolatilityEstimators.hpp"
#include <vector>
#include <cmath>

using namespace AdaptiveExec;

TEST(VolatilityEstimatorsTest, MedRVBasic) {
    // MedRV should handle outliers better than RV
    // Case: 0, 1, 0, 100, 0, 1 ...
    std::vector<Scalar> returns = {0.01, 0.01, 0.01, 0.01, 0.50, 0.01, 0.01};
    // Large jump 0.50 should be filtered out by Median
    
    Scalar med_rv = VolatilityEstimators::computeMedRV(returns);
    Scalar rv = VolatilityEstimators::computeRV(returns);
    
    // MedRV should be significantly smaller than RV due to outlier suppression
    EXPECT_LT(med_rv, rv);
    EXPECT_GT(med_rv, 0.0);
}

TEST(VolatilityEstimatorsTest, LeeMyklandJumpDetection) {
    // Construct a series with a clear jump
    std::vector<Scalar> returns(100, 0.01); // Low vol
    returns[50] = 0.20; // Massive jump at index 50
    
    size_t window = 10;
    auto stats = VolatilityEstimators::computeLeeMykland(returns, window);
    
    ASSERT_EQ(stats.size(), returns.size());
    
    // Check if the jump is detected (stat > 3.0 approx)
    // Local vol is approx 0.01. Jump is 0.20. Stat approx 20.
    EXPECT_GT(stats[50], 5.0);
    
    // Check normal points
    EXPECT_LT(stats[20], 3.0);
}

TEST(VolatilityEstimatorsTest, BipowerVariationRobustness) {
    std::vector<Scalar> returns = {0.01, 0.01, 0.50, 0.01, 0.01};
    Scalar bv = VolatilityEstimators::computeBV(returns);
    Scalar rv = VolatilityEstimators::computeRV(returns);
    
    // BV should be less than RV in presence of jump
    EXPECT_LT(bv, rv);
}
