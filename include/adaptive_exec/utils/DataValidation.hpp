#pragma once

#include "../Types.hpp"
#include <vector>
#include <string>

namespace AdaptiveExec {

    class DataValidator {
    public:
        // Check if a vector contains NaN or Inf
        static bool hasNaNOrInf(const Vector& data);
        
        // Check if all values are positive (e.g. for prices)
        static bool isPositive(const Vector& data);
        
        // Validate market data integrity
        // Returns true if valid, false otherwise. Populates error_msg if invalid.
        static bool validateMarketData(const Vector& prices, const Vector& volumes, std::string& error_msg);
        
        // Sanitize data: Replace NaNs with last valid value (forward fill)
        // returns number of replacements
        static int sanitizeForwardFill(Vector& data);
    };

}
