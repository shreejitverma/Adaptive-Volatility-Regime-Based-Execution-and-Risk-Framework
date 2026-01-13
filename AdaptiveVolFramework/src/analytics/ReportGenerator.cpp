#include "../include/adaptive_exec/analytics/ReportGenerator.hpp"
#include <iostream>
#include <iomanip>

namespace AdaptiveExec {

    void ReportGenerator::printSummary(const MetricsResult& m) {
        std::cout << "\n=============================================="
                  << std::endl;
        std::cout << "          PERFORMANCE REPORT                  "
                  << std::endl;
        std::cout << "=============================================="
                  << std::endl;
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << " Total Return:      " << m.total_return * 100.0 << "%" << std::endl;
        std::cout << " CAGR:              " << m.cagr * 100.0 << "%" << std::endl;
        std::cout << " Annualized Vol:    " << m.annualized_vol * 100.0 << "%" << std::endl;
        std::cout << " Sharpe Ratio:      " << m.sharpe_ratio << std::endl;
        std::cout << " Sortino Ratio:     " << m.sortino_ratio << std::endl;
        std::cout << " Max Drawdown:      " << m.max_drawdown * 100.0 << "%" << std::endl;
        std::cout << " Win Rate:          " << m.win_rate * 100.0 << "%" << std::endl;
        std::cout << "==============================================\n" << std::endl;
    }

}
