#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    class HARModel {
    public:
        HARModel();

        // Fit HAR-RV-J model using OLS
        // rv: Realized Variance series
        // rj: Realized Jumps series
        // returns R-squared
        double fit(const Vector& rv, const Vector& rj);

        // Predict next day volatility
        // current_rv: RV series up to today
        // current_rj: RJ series up to today
        Scalar predict(const Vector& rv, const Vector& rj);

        Vector getCoefficients() const;

    private:
        Vector coefficients_; // [Intercept, Daily, Weekly, Monthly, Jumps]
        bool is_fitted_;

        // Helper to create feature matrix [1, RV_d, RV_w, RV_m, RJ_d]
        std::pair<Matrix, Vector> createFeatures(const Vector& rv, const Vector& rj);
    };

}
