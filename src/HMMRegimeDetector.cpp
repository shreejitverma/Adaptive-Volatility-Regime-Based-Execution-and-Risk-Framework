#include "../include/adaptive_exec/HMMRegimeDetector.hpp"
#include <cmath>
#include <iostream>
#include <limits>

namespace AdaptiveExec {

    HMMRegimeDetector::HMMRegimeDetector(int n_states) 
        : n_states_(n_states), n_features_(0) {}

    void HMMRegimeDetector::setParameters(const Vector& start_prob, const Matrix& trans_mat, const Matrix& means, const Matrix& variances) {
        start_prob_ = start_prob;
        trans_mat_ = trans_mat;
        means_ = means;
        variances_ = variances;
        n_features_ = means.cols();
        
        // Precompute precision matrices and log determinants
        precision_mats_.resize(n_states_);
        log_dets_.resize(n_states_);
        
        for (int i = 0; i < n_states_; ++i) {
            // Extract covariance for this state
            Matrix cov = variances_.block(i * n_features_, 0, n_features_, n_features_);
            
            // Compute Inverse
            precision_mats_[i] = cov.inverse();
            
            // Compute Log Determinant
            Scalar det = cov.determinant();
            if (det <= 0) det = 1e-6; 
            log_dets_[i] = std::log(det);
        }
    }
    
    // Helper for Multi-variate Gaussian Log PDF
    Scalar HMMRegimeDetector::logEmissionProb(int state, const RowVector& x) {
        Vector mu = means_.row(state);
        // Use precomputed values
        const Matrix& invCov = precision_mats_[state];
        Scalar logDet = log_dets_[state];
        
        // Calculate (x - mu)
        Vector diff = x.transpose() - mu;
        
        // Mahalanobis distance term: (x-mu)^T * inv(Cov) * (x-mu)
        Scalar term1 = diff.transpose() * invCov * diff;
        
        Scalar constTerm = n_features_ * std::log(2 * M_PI);
        
        return -0.5 * (constTerm + logDet + term1);
    }

    std::vector<int> HMMRegimeDetector::predictStates(const Matrix& observations) {
        long T = observations.rows();
        if (T == 0) return {};

        // Viterbi Algorithm
        Matrix delta(T, n_states_);
        Matrix psi(T, n_states_); 
        
        // Init (t=0)
        Vector log_start = start_prob_.array().log();
        for (int i = 0; i < n_states_; ++i) {
            delta(0, i) = log_start(i) + logEmissionProb(i, observations.row(0));
        }

        // Recursion
        for (int t = 1; t < T; ++t) {
            for (int j = 0; j < n_states_; ++j) {
                Scalar max_prob = -std::numeric_limits<Scalar>::infinity();
                int best_prev = 0;
                
                Scalar log_emit = logEmissionProb(j, observations.row(t));

                for (int i = 0; i < n_states_; ++i) {
                    Scalar log_trans = std::log(trans_mat_(i, j) + 1e-9); 
                    Scalar prob = delta(t-1, i) + log_trans + log_emit;
                    
                    if (prob > max_prob) {
                        max_prob = prob;
                        best_prev = i;
                    }
                }
                
                delta(t, j) = max_prob;
                psi(t, j) = (Scalar)best_prev;
            }
        }

        // Backtracking
        std::vector<int> states(T);
        Scalar max_final = -std::numeric_limits<Scalar>::infinity();
        int best_final_state = 0;
        for (int i = 0; i < n_states_; ++i) {
            if (delta(T-1, i) > max_final) {
                max_final = delta(T-1, i);
                best_final_state = i;
            }
        }
        states[T-1] = best_final_state;
        
        for (int t = T - 2; t >= 0; --t) {
            states[t] = (int)psi(t+1, states[t+1]);
        }
        
        return states;
    }

    Matrix HMMRegimeDetector::predictProba(const Matrix& observations) {
        long T = observations.rows();
        Matrix alpha(T, n_states_);
        
        for (int i = 0; i < n_states_; ++i) {
            alpha(0, i) = start_prob_(i) * std::exp(logEmissionProb(i, observations.row(0)));
        }
        
        for (int t = 1; t < T; ++t) {
            for (int j = 0; j < n_states_; ++j) {
                Scalar sum = 0.0;
                for (int i = 0; i < n_states_; ++i) {
                    sum += alpha(t-1, i) * trans_mat_(i, j);
                }
                alpha(t, j) = sum * std::exp(logEmissionProb(j, observations.row(t)));
            }
            Scalar row_sum = alpha.row(t).sum();
            if (row_sum > 0) alpha.row(t) /= row_sum;
        }
        
        return alpha;
    }

    void HMMRegimeDetector::fit(const Matrix& observations) {
        std::cout << "Training logic would go here (Baum-Welch)." << std::endl;
    }

}