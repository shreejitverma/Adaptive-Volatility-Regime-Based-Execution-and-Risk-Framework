#include "../include/adaptive_exec/HARModel.hpp"
#include <iostream>
#include <numeric>

namespace AdaptiveExec {

    HARModel::HARModel() : is_fitted_(false) {
        coefficients_ = Vector::Zero(5);
    }

    std::pair<Matrix, Vector> HARModel::createFeatures(const Vector& rv, const Vector& rj) {
        // Python:
        // rv_d = rv[22:] (t-1)
        // rv_w = mean(rv[i-5:i])
        // rv_m = mean(rv[i-22:i])
        // rj_d = rj[22:]
        // y = rv[23:] (target is next day) -> Wait, python code says:
        // X from 22 to end-1, y from 23 to end.
        
        // Let's replicate logic:
        // We need indices such that we have 22 days of history.
        // Start predicting at index 22 (using 0..21).
        // Actually, Python code:
        // rv_d = rv[22:]  -- starts at index 22
        // rv_w loop i from 22 to n
        // y = rv[23:]
        // This implies X[i] predicts y[i] where y[i] is effectively rv[i+1] relative to X's time?
        // Let's trace carefully.
        // n = len(rv)
        // loops i from 22 to n.
        // rv_d element is rv[i] (which is the previous day relative to target? No, standard HAR predicts t+1 using t, t-1..t-4, etc.)
        // If i=22, rv[22] is the daily lag.
        // rv_w is mean(17..22).
        // Target is rv[23].
        
        // So we build X and y.
        long n = rv.size();
        long start_idx = 22;
        long n_samples = n - start_idx - 1; // -1 because we need target at i+1
        
        if (n_samples <= 0) {
            return {Matrix(0, 0), Vector(0)};
        }

        Matrix X(n_samples, 5); // Intercept, D, W, M, J
        Vector y(n_samples);

        for (long i = 0; i < n_samples; ++i) {
            long curr = start_idx + i;
            
            // Daily: rv[curr]
            Scalar val_d = rv[curr];
            
            // Weekly: mean(rv[curr-5 : curr]) -> indices curr-5, ..., curr-1
            Scalar sum_w = 0.0;
            for(int k=1; k<=5; ++k) sum_w += rv[curr - k];
            Scalar val_w = sum_w / 5.0;

            // Monthly: mean(rv[curr-22 : curr])
            Scalar sum_m = 0.0;
            for(int k=1; k<=22; ++k) sum_m += rv[curr - k];
            Scalar val_m = sum_m / 22.0;
            
            // Jumps: rj[curr]
            Scalar val_j = rj[curr];

            X(i, 0) = 1.0; // Intercept
            X(i, 1) = val_d;
            X(i, 2) = val_w;
            X(i, 3) = val_m;
            X(i, 4) = val_j;

            y(i) = rv[curr + 1];
        }

        return {X, y};
    }

    double HARModel::fit(const Vector& rv, const Vector& rj) {
        auto data = createFeatures(rv, rj);
        const Matrix& X = data.first;
        const Vector& y = data.second;

        if (X.rows() == 0) return 0.0;

        // OLS: coeff = (X^T X)^-1 X^T y
        // Using QR decomposition for stability
        coefficients_ = X.colPivHouseholderQr().solve(y);
        is_fitted_ = true;

        // R-squared
        Vector predictions = X * coefficients_;
        Vector residuals = y - predictions;
        Scalar ss_res = residuals.squaredNorm();
        Scalar mean_y = y.mean();
        Vector centered_y = y.array() - mean_y;
        Scalar ss_tot = centered_y.squaredNorm();

        return 1.0 - (ss_res / ss_tot);
    }

    Scalar HARModel::predict(const Vector& rv, const Vector& rj) {
        if (!is_fitted_) return 0.0;
        
        long n = rv.size();
        if (n < 22) return 0.0; // Not enough history

        // Prepare feature vector for the *next* day
        // Uses the *last* available data points
        long curr = n - 1;
        
        Scalar val_d = rv[curr];
        
        Scalar sum_w = 0.0;
        for(int k=0; k<5; ++k) sum_w += rv[curr - k]; // curr, curr-1...
        // Wait, the logic in createFeatures for 'weekly' was mean(rv[curr-5:curr]) which EXCLUDES curr in Python slice?
        // Python: rv[i-5:i] -> range [i-5, i-1]. Yes.
        // So for prediction at time t+1, we use info at time t.
        // Daily: RV_t
        // Weekly: Average RV_{t-4}..RV_{t} ?
        // Python create_features:
        // rv_d = rv[i] (where i goes from 22..)
        // rv_w = mean(rv[i-5:i]) -> rv[i-5]...rv[i-1]
        // This effectively means Daily is 'today', Weekly is 'past week average excluding today'.
        // Is that standard HAR? Standard HAR usually includes today in the averages (Corsi 2009).
        // "Weekly is average of t-1 to t-5" says the comment in notebook.
        // Code: rv_w = mean(rv[i-5:i]). If i is today, slice i-5:i gets i-5, i-4, i-3, i-2, i-1.
        // Yes, it excludes i.
        // So Daily term is Lag 1. Weekly term is Lag 2-6 average. Monthly is Lag 2-23.
        
        // Let's stick to Python implementation.
        
        // Logic for PREDICT (at time T, predict T+1):
        // We need:
        // D: rv[T]
        // W: mean(rv[T-5 : T]) -> rv[T-5]...rv[T-1]
        // M: mean(rv[T-22 : T]) -> rv[T-22]...rv[T-1]
        // J: rj[T]
        
        Scalar sum_w_lag = 0.0;
        for(int k=1; k<=5; ++k) sum_w_lag += rv[curr - k];
        Scalar val_w = sum_w_lag / 5.0;

        Scalar sum_m_lag = 0.0;
        for(int k=1; k<=22; ++k) sum_m_lag += rv[curr - k];
        Scalar val_m = sum_m_lag / 22.0;
        
        Scalar val_j = rj[curr];

        Vector x(5);
        x << 1.0, val_d, val_w, val_m, val_j;

        return x.dot(coefficients_);
    }
    
    Vector HARModel::getCoefficients() const {
        return coefficients_;
    }

}
