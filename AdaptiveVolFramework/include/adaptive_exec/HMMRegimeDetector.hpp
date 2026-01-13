#pragma once

#include "Types.hpp"
#include <vector>

namespace AdaptiveExec {

    /**
     * @class HMMRegimeDetector
     * @brief Implements a Gaussian Hidden Markov Model (HMM) for market regime classification.
     * 
     * This class uses the Viterbi algorithm to decode the most likely sequence of hidden
     * market states (e.g., Low Vol, Normal, High Vol) based on observed market features
     * such as TSRV (Two-Scale Realized Volatility) and Realized Jumps.
     * 
     * It supports precomputed precision matrices and log-determinants for high-performance
     * inference suitable for HFT environments.
     */
    class HMMRegimeDetector {
    public:
        /**
         * @brief Construct a new HMMRegimeDetector object
         * 
         * @param n_states Number of hidden states (default: 3)
         */
        HMMRegimeDetector(int n_states = 3);

        /**
         * @brief Set the Model Parameters manually.
         * 
         * Ideally, these parameters are learned offline (e.g., using Python/hmmlearn) 
         * and loaded here for real-time inference.
         * 
         * @param start_prob Initial state probabilities (size: n_states)
         * @param trans_mat Transition probability matrix (n_states x n_states)
         * @param means Mean vectors for each state (n_states x n_features)
         * @param variances Covariance matrices (stacked: (n_states * n_features) x n_features)
         */
        void setParameters(const Vector& start_prob, const Matrix& trans_mat, const Matrix& means, const Matrix& variances);

        /**
         * @brief Decode the most likely sequence of states for a given observation sequence.
         * 
         * Uses the Viterbi Algorithm (Dynamic Programming).
         * Time Complexity: O(T * N^2), where T is time steps and N is number of states.
         * 
         * @param observations Matrix of shape (Time x Features)
         * @return std::vector<int> Sequence of state indices (0 to n_states-1)
         */
        std::vector<int> predictStates(const Matrix& observations);

        /**
         * @brief Calculate posterior state probabilities.
         * 
         * Uses the Forward algorithm logic to compute P(State_t | Observations).
         * 
         * @param observations Matrix of shape (Time x Features)
         * @return Matrix Probabilities of shape (Time x States)
         */
        Matrix predictProba(const Matrix& observations);

        /**
         * @brief Train the model parameters (Placeholder).
         * 
         * In a full implementation, this would use the Baum-Welch (EM) algorithm.
         * Currently used for interface compatibility.
         * 
         * @param observations Training data
         */
        void fit(const Matrix& observations);

    private:
        int n_states_;
        int n_features_;
        
        Vector start_prob_;
        Matrix trans_mat_;
        Matrix means_;
        Matrix variances_; 
        
        // Precomputed for speed
        std::vector<Matrix> precision_mats_; // Inverse of covariances
        Vector log_dets_; // Log determinants of covariances

        /**
         * @brief Compute Log-Likelihood of an observation given a state.
         * Uses precomputed precision matrices for O(D^2) efficiency.
         */
        Scalar logEmissionProb(int state, const RowVector& x);
    };

}
