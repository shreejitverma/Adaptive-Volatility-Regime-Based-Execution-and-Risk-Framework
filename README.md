# Adaptive Volatility Regime-Based Execution & Risk Framework

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A high-performance C++ framework designed for **High-Frequency Trading (HFT)** and quantitative research. This system dynamically adapts execution algorithms and risk parameters by identifying latent market regimes (Low, Normal, High Volatility) in real-time.

It integrates advanced econometric models (**TSRV**, **MedRV**, **HAR**) with machine learning (**HMM**) and self-exciting point processes (**Hawkes**) to provide a robust safety net for automated trading strategies.

---

## 🚀 Key Features

### 1. Microstructure-Robust Volatility & Jump Estimation
*   **Two-Scale Realized Volatility (TSRV):** Mitigates microstructure noise (bid-ask bounce) by subsampling high-frequency returns and bias-correcting.
    *   *Formula:* $TSRV = \left(1 - \frac{\bar{n}_K}{n}\right)^{-1} \left(RV_{avg}^{(K)} - \frac{\bar{n}_K}{n} RV_{all}\right)$
*   **Median Integrated Volatility (MedRV):** A robust estimator that filters out jumps, providing a cleaner signal for continuous volatility than standard Realized Volatility.
    *   *Formula:* $MedRV_t = \frac{\pi}{6-4\sqrt{3}+\pi} \frac{N}{N-2} \sum_{i=2}^{N} \text{med}(|r_i|, |r_{i-1}|, |r_{i-2}|)^2$
*   **Lee-Mykland Jump Detection:** Statistical test to identify significant price jumps in real-time using local volatility estimates (Bipower Variation).
    *   *Criterion:* Jump if $\frac{|r_t|}{\hat{\sigma}_t} > C_{\text{thresh}}$ (typically 3.0-3.5).

### 2. Regime Classification (Machine Learning)
*   **Gaussian HMM:** Implements a 3-State Hidden Markov Model to classify market states into **Low**, **Normal**, and **High** volatility regimes.
*   **Viterbi Decoding:** Uses dynamic programming to decode the most likely sequence of hidden states in $O(T \cdot N^2)$ time.
*   **Log-Normal Features:** Transforms raw volatility inputs into log-space to better approximate Gaussian distributions required by the HMM.

### 3. Forecasting & Alpha
*   **HAR-RV-J Model:** A Heterogeneous Autoregressive model capturing volatility clustering across Daily, Weekly, and Monthly horizons, with explicit jump components.
    *   *Model:* $\log(\sigma_{t+1}) = \beta_0 + \beta_d \log(RV_t) + \beta_w \log(RV_{t-1:t-5}) + \beta_m \log(RV_{t-1:t-22}) + \beta_j \log(1 + J_t) + \epsilon$

### 4. Ultra-HFT Safety (Circuit Breakers)
*   **Hawkes Processes:** Models trade arrival intensity as a self-exciting point process to detect flash crashes and liquidity vacuums.
    *   *Intensity:* $\lambda(t) = \mu + \sum_{t_i < t} \alpha e^{-\beta(t - t_i)}$
*   **Reaction Speed:** Detects intensity spikes in sub-millisecond timeframes to trigger emergency circuit breakers (halting or reducing trade size).

### 5. Adaptive Execution & Risk
*   **Dynamic Sizing:** Position sizes are scaled inversely to the regime risk (1.5x in Low Vol, 0.5x in High Vol).
*   **Cost Modeling:** Incorporates regime-dependent spread and square-root market impact models ($Cost \propto \sigma \cdot \sqrt{Q}$). 
*   **Execution Scheduling:** Switches between **VWAP** (Aggressive), **TWAP** (Neutral), and **Passive** (Defensive) schedules based on the active regime.

---

## 🏗 System Architecture

The project is modularized into independent components linked via the `AdaptiveVolCore` library.

```
.
├── include/adaptive_exec/       # Public API Headers
│   ├── VolatilityEstimators.hpp # TSRV, MedRV, Lee-Mykland Logic
│   ├── HMMRegimeDetector.hpp    # Regime Classification (Viterbi)
│   ├── HARModel.hpp             # Volatility Forecasting
│   ├── HawkesModel.hpp          # HFT Circuit Breakers
│   ├── ExecutionEngine.hpp      # VWAP/TWAP & Cost Logic
│   ├── RiskManager.hpp          # Position Sizing & CVaR
│   └── backtest/                # Simulation Engine
├── src/                         # Implementation (Eigen3 optimized)
├── tests/                       # GoogleTest Suite
└── AdaptiveVolDemo              # End-to-End Simulation Executable
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
    git clone https://github.com/your-repo/Adaptive-Volatility-Regime-Based-Execution-and-Risk-Framework.git
    cd Adaptive-Volatility-Regime-Based-Execution-and-Risk-Framework
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

### 1. Estimating Robust Volatility
```cpp
#include <adaptive_exec/VolatilityEstimators.hpp>

// High-freq returns vector
std::vector<double> returns = { ... };

// Compute Median Integrated Volatility (Jump Robust)
double med_rv = VolatilityEstimators::computeMedRV(returns);

// Detect Jumps
auto jump_stats = VolatilityEstimators::computeLeeMykland(returns);
if (jump_stats.back() > 3.0) {
    // Jump detected!
}
```

### 2. Detecting Market Regimes
```cpp
#include <adaptive_exec/HMMRegimeDetector.hpp>

// Initialize 3-State Model
HMMRegimeDetector hmm(3);

// Decode Regime (0=Low, 1=Normal, 2=High)
std::vector<int> states = hmm.predictStates(observations);
```

### 3. Monitoring HFT Intensity (Hawkes)
```cpp
#include <adaptive_exec/HawkesModel.hpp>

HawkesModel hawkes(0.5, 0.8, 1.0); // Baseline, Jump, Decay

// On every trade arrival:
double intensity = hawkes.addEvent(current_timestamp);

if (ExecutionEngine::checkCircuitBreaker(intensity, 5.0)) {
    // Halt Trading
}
```

---

## 📊 Backtesting Performance

The simulation engine generates synthetic regime-switching data with drift and jumps to validate the strategy. Recent benchmarks show strong risk-adjusted returns:

```text
==============================================
          PERFORMANCE REPORT                  
==============================================
 Total Return:      53.53%
 CAGR:              70.72%
 Annualized Vol:    39.02%
 Sharpe Ratio:      1.57
 Sortino Ratio:     2.41
 Max Drawdown:      21.42%
 Win Rate:          57.21%
==============================================
```

*Note: Performance depends on synthetic data parameters (drift, volatility, jump frequency).*

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
