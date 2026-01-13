#include "../include/adaptive_exec/backtest/BacktestEngine.hpp"
#include <iostream>

namespace AdaptiveExec {

    BacktestEngine::BacktestEngine(Scalar initial_capital) {
        reset(initial_capital);
    }

    void BacktestEngine::reset(Scalar initial_capital) {
        initial_capital_ = initial_capital;
        cash_ = initial_capital;
        position_ = 0.0;
        trades_.clear();
        // Reserve some space to avoid reallocs
        // equity_curve_ is dynamic, handled via conservative resizing or just appending to std::vector then converting?
        // Eigen Vector is fixed size if usually not resized incrementally efficiently.
        // For simplicity in this demo, let's use std::vector internally for building curve then export to Eigen.
    }
    
    // Helper to append to Eigen Vector (inefficient but simple for demo)
    void appendToVector(Vector& v, Scalar val) {
        long n = v.size();
        v.conservativeResize(n + 1);
        v(n) = val;
    }

    void BacktestEngine::executeOrder(int day, Scalar price, Scalar quantity, MarketRegime regime) {
        if (std::abs(quantity) < 1e-6) return; // No trade

        // Calculate Cost
        // Cost in bps relative to notional value
        Scalar notional = price * std::abs(quantity);
        Scalar cost_bps = ExecutionEngine::computeTransactionCosts(regime, std::abs(quantity));
        Scalar transaction_cost = notional * (cost_bps / 10000.0);

        // Update Portfolio
        cash_ -= (price * quantity); // Buy: cash down. Sell: cash up.
        cash_ -= transaction_cost;   // Always pay cost
        position_ += quantity;

        // Log Trade
        Trade t;
        t.day = day;
        t.price = price;
        t.quantity = quantity;
        t.cost = transaction_cost;
        t.pnl = 0; // Realized PnL logic is complex (FIFO/LIFO), skipping for Mark-to-Market focus
        trades_.push_back(t);
    }

    void BacktestEngine::updateEndOfDay(Scalar close_price) {
        Scalar equity = cash_ + (position_ * close_price);
        appendToVector(equity_curve_, equity);
    }

    Vector BacktestEngine::getEquityCurve() const {
        return equity_curve_;
    }

    std::vector<Trade> BacktestEngine::getTrades() const {
        return trades_;
    }

    MetricsResult BacktestEngine::getPerformanceMetrics() const {
        return PerformanceMetrics::calculate(equity_curve_);
    }

}
