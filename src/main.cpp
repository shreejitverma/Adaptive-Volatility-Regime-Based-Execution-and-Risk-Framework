#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include "../include/adaptive_exec/VolatilityEstimators.hpp"
#include "../include/adaptive_exec/HARModel.hpp"
#include "../include/adaptive_exec/HMMRegimeDetector.hpp"
#include "../include/adaptive_exec/ExecutionEngine.hpp"
#include "../include/adaptive_exec/RiskManager.hpp"
#include "../include/adaptive_exec/HawkesModel.hpp"
#include "../include/adaptive_exec/backtest/BacktestEngine.hpp"
#include "../include/adaptive_exec/analytics/ReportGenerator.hpp"

using namespace AdaptiveExec;

// --- Helper Functions ---

// Generate synthetic daily features (Continuous Vol + Jumps) +PRICES
void generateSyntheticData(int n_days, Vector& tsrv, Vector& rj, Vector& prices) {
    tsrv.resize(n_days);
    rj.resize(n_days);
    prices.resize(n_days);
    
    std::mt19937 gen(42);
    // Log-Normal distributions for more realism
    std::lognormal_distribution<> d_low(std::log(6.0), 0.3);
    std::lognormal_distribution<> d_normal(std::log(50.0), 0.5);
    std::lognormal_distribution<> d_high(std::log(90.0), 0.8);
    
    std::exponential_distribution<> jump_dist(1.0);
    std::normal_distribution<> price_noise(0.0, 1.0);
    
    int current_regime = 0; 
    prices[0] = 100.0;

    for (int i = 0; i < n_days; ++i) {
        if (i % 100 == 0) current_regime = (current_regime + 1) % 3;
        
        double vol_scale = 1.0;

        if (current_regime == 0) {
            tsrv[i] = d_low(gen);
            rj[i] = d_low(gen) * 0.05; 
            vol_scale = 0.5;
        } else if (current_regime == 1) {
            tsrv[i] = d_normal(gen);
            rj[i] = d_normal(gen) * 0.2 + jump_dist(gen)*5;
            vol_scale = 1.0;
        } else {
            tsrv[i] = d_high(gen);
            rj[i] = d_high(gen) * 0.5 + jump_dist(gen)*20;
            vol_scale = 2.0;
        }

        // Generate Price Walk (Geometric Brownian Motion approx)
        if (i > 0) {
            double ret = price_noise(gen) * (vol_scale / 100.0);
            prices[i] = prices[i-1] * std::exp(ret);
        }
    }
}

// Simulate Intraday "Tick" Flow for HFT Demo
// Returns number of circuit breaker triggers
int simulateIntradaySession(int day, MarketRegime daily_regime) {
    double mu = (daily_regime == MarketRegime::HighVolatility) ? 2.0 : 0.5;
    double alpha = (daily_regime == MarketRegime::HighVolatility) ? 0.8 : 0.2;
    double beta = 1.0; 
    double threshold = 5.0; // Circuit breaker limit

    HawkesModel hawkes(mu, alpha, beta);

    std::mt19937 gen(day * 1234); 
    std::exponential_distribution<> arrival_dist(mu); 
    
    double t = 0.0;
    double session_end = 100.0; 
    int triggers = 0;
    
    while (t < session_end) {
        double dt = arrival_dist(gen);
        if (daily_regime == MarketRegime::HighVolatility && (int)t % 20 == 0) {
            dt = 0.01; 
        }

        t += dt;
        if (t > session_end) break;

        double intensity = hawkes.addEvent(t);
        if (ExecutionEngine::checkCircuitBreaker(intensity, threshold)) {
            triggers++;
        }
    }
    return triggers;
}


int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << " ULTRA-HFT ADAPTIVE FRAMEWORK (MARKET READY)" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "Modules: Backtesting Engine, Performance Analytics, Reporting," << std::endl;
    std::cout << "         Data Validation, Unit Tests" << std::endl;
    std::cout << "==========================================================\n" << std::endl;

    // 1. Generate Data
    int n_days = 252; // Full year
    Vector tsrv, rj, prices;
    generateSyntheticData(n_days, tsrv, rj, prices);
    std::cout << "[Data] Generated " << n_days << " days of regime-switching market data & prices." << std::endl;

    // 2. Pre-process for HMM: LOG TRANSFORMATION
    Vector log_tsrv = tsrv.array().log();
    Vector log_rj = (rj.array() + 1e-6).log();

    // 3. Setup HMM (Trained on Log-Features)
    HMMRegimeDetector hmm(3);
    Vector start_prob(3); start_prob << 0.5, 0.3, 0.2;
    Matrix trans_mat(3, 3);
    trans_mat << 0.95, 0.04, 0.01,
                 0.05, 0.90, 0.05,
                 0.01, 0.10, 0.89;
    Matrix means(3, 2); 
    means << std::log(6.0), std::log(0.6),    // Low
             std::log(53.0), std::log(13.0),  // Normal
             std::log(88.0), std::log(53.0);  // High
    Matrix variances(3 * 2, 2); 
    variances.row(0) << 0.2, 0.0; variances.row(1) << 0.0, 1.0; 
    variances.row(2) << 0.5, 0.0; variances.row(3) << 0.0, 1.5; 
    variances.row(4) << 0.8, 0.0; variances.row(5) << 0.0, 2.0; 

    hmm.setParameters(start_prob, trans_mat, means, variances);
    std::cout << "[Model] HMM initialized." << std::endl;

    // 4. Setup HAR Model
    HARModel har;
    Vector train_rv = tsrv.head(50);
    Vector train_rj = rj.head(50);
    har.fit(train_rv, train_rj);
    std::cout << "[Model] HAR Forecasting Model Fitted." << std::endl;

    // 5. Initialize Backtest Engine
    BacktestEngine backtester(100000.0);
    std::cout << "[Backtest] Engine initialized with $100,000 capital." << std::endl;

    // 6. Simulation Loop
    std::cout << "\n[Starting HFT Backtest Simulation...]" << std::endl;
    
    // Decode states
    Matrix observations(n_days, 2);
    observations.col(0) = log_tsrv;
    observations.col(1) = log_rj;
    std::vector<int> states = hmm.predictStates(observations);

    for (int i = 50; i < n_days; ++i) {
        // --- Macro Layer ---
        int state_idx = states[i];
        MarketRegime regime = static_cast<MarketRegime>(state_idx);

        Vector hist_rv = tsrv.head(i+1);
        Vector hist_rj = rj.head(i+1);
        // Scalar vol_forecast = har.predict(hist_rv, hist_rj); // Unused in this demo strategy logic

        // Simple Trend Signal (SMA Crossover) for direction
        Scalar quantity = 0.0;
        if (i > 20) {
            Scalar sma_short = prices.segment(i-5, 5).mean();
            Scalar sma_long = prices.segment(i-20, 20).mean();
            
            // Risk Sizing
            Scalar base_qty = 100.0; // lots
            Scalar sized_qty = RiskManager::getRegimePositionSize(regime, base_qty);
            
            if (sma_short > sma_long) quantity = sized_qty;
            else quantity = -sized_qty;
        }

        // --- Micro Layer Checks ---
        int safety_triggers = simulateIntradaySession(i, regime);
        if (safety_triggers > 400) { // arbitrary threshold for demo
             // Circuit Breaker: Reduce or Cancel Trade
             quantity = 0.0; 
        }

        // Execute Strategy
        backtester.executeOrder(i, prices[i], quantity, regime);
        backtester.updateEndOfDay(prices[i]);
    }
    
    // 7. Reporting
    MetricsResult metrics = backtester.getPerformanceMetrics();
    ReportGenerator::printSummary(metrics);
    
    return 0;
}
