#include "../../include/adaptive_exec/backtest/BacktestEngine.hpp"
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
        equity_curve_.clear();
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
        equity_curve_.push_back(equity);
    }

    Vector BacktestEngine::getEquityCurve() const {
        // Convert std::vector to Eigen::VectorXd
        Vector v(equity_curve_.size());
        for (size_t i = 0; i < equity_curve_.size(); ++i) {
            v(i) = equity_curve_[i];
        }
        return v;
    }

    std::vector<Trade> BacktestEngine::getTrades() const {
        return trades_;
    }

    MetricsResult BacktestEngine::getPerformanceMetrics() const {
        return PerformanceMetrics::calculate(getEquityCurve());
    }

}