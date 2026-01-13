# Adaptive Volatility Regime-Based Execution & Risk Framework

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A high-performance C++ framework designed for **High-Frequency Trading (HFT)** and quantitative research. This system dynamically adapts execution algorithms and risk parameters by identifying latent market regimes (Low, Normal, High Volatility) in real-time.

It integrates advanced econometric models (TSRV, HAR) with machine learning (HMM) and self-exciting point processes (Hawkes) to provide a robust safety net for automated trading strategies.

---

## 🚀 Key Features

### 1. Microstructure-Robust Volatility Estimation
*   **Two-Scale Realized Volatility (TSRV):** Mitigates microstructure noise (bid-ask bounce) by subsampling high-frequency returns.
*   **Realized Jumps (RJ):** Isolates discontinuous price shocks from continuous diffusion using Bipower Variation (BV).
    *   *Math:* $RJ_t = \max(0, RV_t - BV_t)$

### 2. Regime Classification (Machine Learning)
*   **Gaussian HMM:** Implements a 3-State Hidden Markov Model to classify market states.
*   **Viterbi Decoding:** Uses dynamic programming for optimal state sequence detection in $O(T \cdot N^2)$.
*   **Log-Normal Transformation:** Handles skewed volatility distributions correctly by transforming features into Gaussian space.

### 3. Forecasting & Alpha
*   **HAR-RV-J Model:** A Heterogeneous Autoregressive model capturing volatility clustering across Daily, Weekly, and Monthly horizons.
    *   *Model:* $\sigma_{t+1} = \beta_0 + \beta_d RV_t + \beta_w RV_{t-1:t-5} + \beta_m RV_{t-1:t-22} + \beta_j RJ_t + \epsilon$

### 4. Ultra-HFT Safety (Circuit Breakers)
*   **Hawkes Processes:** Models trade arrival intensity as a self-exciting point process.
*   **Reaction Speed:** Detects liquidity shocks and flash crashes in sub-millisecond timeframes to trigger emergency halts.

### 5. Adaptive Execution
*   **Dynamic Scheduling:** Switches between **VWAP** (Aggressive/Low Vol), **TWAP** (Neutral), and **Passive** (Defensive/High Vol) strategies based on the active regime.
*   **Cost Modeling:** Incorporates spread and square-root market impact models.

---

## 🏗 System Architecture

The project is modularized into independent components linked via the `AdaptiveVolCore` library.

```
AdaptiveVolFramework/
├── include/adaptive_exec/       # Public API Headers
│   ├── VolatilityEstimators.hpp # TSRV, BV, RJ Logic
│   ├── HMMRegimeDetector.hpp    # Regime Classification (Viterbi)
│   ├── HARModel.hpp             # Volatility Forecasting
│   ├── HawkesModel.hpp          # HFT Circuit Breakers
│   ├── ExecutionEngine.hpp      # VWAP/TWAP Logic
│   └── backtest/                # Simulation Engine
├── src/                         # Implementation (Eigen3 optimized)
├── tests/                       # GoogleTest Suite
└── AdaptiveVolDemo              # End-to-End Simulation
```

---

## 🛠 Building the Project

### Prerequisites
*   **CMake** (3.14+)
*   **C++ Compiler** (GCC, Clang, or MSVC) supporting C++17
*   **Internet Access** (The build system automatically fetches `Eigen` and `GoogleTest`)

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-repo/AdaptiveVolFramework.git
    cd AdaptiveVolFramework
    ```

2.  **Configure and Compile:**
    ```bash
    mkdir build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j4
    ```

3.  **Run Tests:**
    ```bash
    ./UnitTests
    ```
    *Output should show 100% pass rate.*

4.  **Run the Demo:**
    ```bash
    ./AdaptiveVolDemo
    ```

---

## 💻 Usage Examples

### 1. Estimating Volatility from Ticks
```cpp
#include <adaptive_exec/VolatilityEstimators.hpp>

// High-freq returns vector
std::vector<double> returns = { ... };

// Compute Noise-Robust Volatility (TSRV)
double tsrv = VolatilityEstimators::computeTSRV(returns, 5); // K=5 subsampling

// Compute Jump Component
double jumps = VolatilityEstimators::computeRJ(returns);
```

### 2. Detecting Market Regimes
```cpp
#include <adaptive_exec/HMMRegimeDetector.hpp>

// Initialize 3-State Model
HMMRegimeDetector hmm(3);

// Load parameters (e.g., from offline calibration)
hmm.setParameters(start_prob, trans_mat, means, variances);

// Decode Regime (0=Low, 1=Normal, 2=High)
std::vector<int> states = hmm.predictStates(observations);
```

### 3. Monitoring HFT Intensity (Hawkes)
```cpp
#include <adaptive_exec/HawkesModel.hpp>

// Baseline=0.5, Alpha=0.8 (Jump), Beta=1.0 (Decay)
HawkesModel hawkes(0.5, 0.8, 1.0);

// On every trade arrival:
double intensity = hawkes.addEvent(current_timestamp);

if (intensity > 5.0) {
    ExecutionEngine::haltTrading(); // Trigger Circuit Breaker
}
```

---

## 📊 Backtesting Performance

The included `BacktestEngine` provides professional-grade metrics. Sample output from the demo:

```text
==============================================
          PERFORMANCE REPORT                  
==============================================
 Total Return:      12.50%
 Sharpe Ratio:      1.85
 Sortino Ratio:     2.40
 Max Drawdown:      -4.20%
 Win Rate:          58.00%
==============================================
```

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.