#include "../include/adaptive_exec/utils/DataValidation.hpp"
#include <cmath>
#include <iostream>

namespace AdaptiveExec {

    bool DataValidator::hasNaNOrInf(const Vector& data) {
        return !data.allFinite();
    }

    bool DataValidator::isPositive(const Vector& data) {
        return (data.array() > 0).all();
    }

    bool DataValidator::validateMarketData(const Vector& prices, const Vector& volumes, std::string& error_msg) {
        if (prices.size() != volumes.size()) {
            error_msg = "Mismatch in Prices and Volumes length.";
            return false;
        }
        if (prices.size() == 0) {
            error_msg = "Empty data provided.";
            return false;
        }
        if (hasNaNOrInf(prices) || hasNaNOrInf(volumes)) {
            error_msg = "Data contains NaN or Inf.";
            return false;
        }
        if (!isPositive(prices)) {
            error_msg = "Prices must be strictly positive.";
            return false;
        }
        // Volumes can be zero (no trade), but not negative
        if ((volumes.array() < 0).any()) {
            error_msg = "Volumes cannot be negative.";
            return false;
        }
        return true;
    }

    int DataValidator::sanitizeForwardFill(Vector& data) {
        int replacements = 0;
        Scalar last_valid = 0.0;
        
        // Find first valid
        long start_idx = 0;
        for (; start_idx < data.size(); ++start_idx) {
            if (std::isfinite(data[start_idx])) {
                last_valid = data[start_idx];
                break;
            }
        }
        
        // Fill forward
        for (long i = start_idx + 1; i < data.size(); ++i) {
            if (!std::isfinite(data[i])) {
                data[i] = last_valid;
                replacements++;
            } else {
                last_valid = data[i];
            }
        }
        
        // Backfill potentially (if starts with NaNs)
        for (long i = 0; i < start_idx; ++i) {
             data[i] = data[start_idx];
             replacements++;
        }
        
        return replacements;
    }

}
