#include <gtest/gtest.h>
#include "../include/adaptive_exec/VolatilityEstimators.hpp"
#include "../include/adaptive_exec/HMMRegimeDetector.hpp"
#include "../include/adaptive_exec/backtest/BacktestEngine.hpp"
#include "../include/adaptive_exec/utils/DataValidation.hpp"

using namespace AdaptiveExec;

TEST(IntegrationTest, FullPipelineExecution) {
    // 1. Setup Mock Data
    int n_days = 60;
    Vector prices(n_days);
    Vector tsrv(n_days);
    Vector rj(n_days);
    
    for(int i=0; i<n_days; ++i) {
        prices[i] = 100.0 + i*0.1;
        tsrv[i] = 10.0;
        rj[i] = 0.5;
    }

    // 2. Data Validation
    std::string err;
    Vector volumes = Vector::Ones(n_days);
    bool valid = DataValidator::validateMarketData(prices, volumes, err);
    EXPECT_TRUE(valid);

    // 3. Regime Detection (Simple Params)
    HMMRegimeDetector hmm(2);
    Vector start(2); start << 0.5, 0.5;
    Matrix trans(2,2); trans << 0.9, 0.1, 0.1, 0.9;
    Matrix means(2,2); means << 2.0, 0.0, 4.0, 2.0; // Log space
    Matrix vars(4,2); vars.setIdentity();

    hmm.setParameters(start, trans, means, vars);
    
    Matrix obs(n_days, 2);
    obs.col(0) = tsrv.array().log();
    obs.col(1) = (rj.array() + 1e-6).log();
    
    std::vector<int> states = hmm.predictStates(obs);
    EXPECT_EQ(states.size(), n_days);

    // 4. Backtest
    BacktestEngine bt(1000.0);
    for(int i=0; i<n_days; ++i) {
        bt.executeOrder(i, prices[i], 1.0, static_cast<MarketRegime>(states[i]));
        bt.updateEndOfDay(prices[i]);
    }

    Vector equity = bt.getEquityCurve();
    EXPECT_EQ(equity.size(), n_days);
    EXPECT_GT(equity[n_days-1], 0.0);
}
