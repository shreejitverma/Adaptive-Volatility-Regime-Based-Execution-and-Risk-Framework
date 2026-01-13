#pragma once

#include "PerformanceMetrics.hpp"
#include <string>

namespace AdaptiveExec {

    class ReportGenerator {
    public:
        static void printSummary(const MetricsResult& metrics);
    };

}
