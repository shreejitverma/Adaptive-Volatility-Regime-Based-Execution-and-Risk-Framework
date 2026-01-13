# Adaptive Volatility Regime-Based Execution & Risk Framework

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A high-performance C++ framework designed for **High-Frequency Trading (HFT)** and quantitative research. This system dynamically adapts execution algorithms and risk parameters by identifying latent market regimes (Low, Normal, High Volatility) in real-time.

It integrates advanced econometric models (**TSRV**, **MedRV**, **HAR**) with machine learning (**HMM**) and self-exciting point processes (**Hawkes**) to provide a robust safety net for automated trading strategies.

---

## Key Features

### 1. Volatility and Jump Estimation

The framework provides a suite of advanced estimators to handle the complexities of high-frequency financial data, including microstructure noise and price jumps.

*   **Two-Scale Realized Volatility (TSRV):**
    A robust estimator that mitigates microstructure noise by subsampling high-frequency returns and applying a bias correction.

    $$
    \text{TSRV} = \left(1 - \frac{\bar{n}_K}{n}\right)^{-1} \left(RV_{avg}^{(K)} - \frac{\bar{n}_K}{n} RV_{all}\right)
    $$
    
    *Where:*
    - `$n$` is the total number of observations.
    - `$K$` is the subsampling frequency.
    - `$RV_{all}$` is the standard Realized Volatility on all returns.
    - `$RV_{avg}^{(K)}$` is the average RV across `$K$` subsampled grids.

*   **Median Integrated Volatility (MedRV):**
    A powerful estimator that is highly robust to price jumps, providing a cleaner signal of the market's continuous volatility.

    $$
    \text{MedRV}_t = \frac{\pi}{6-4\sqrt{3}+\pi} \frac{N}{N-2} \sum_{i=2}^{N} \text{med}(|r_i|, |r_{i-1}|, |r_{i-2}|)^2
    $$

    *Where:*
    - `$N$` is the number of returns.
    - `$r_i$` is the return at time `$i$`.
    - `med()` is the median function over a local window of three returns.

*   **Lee-Mykland Jump Detection:**
    A statistical test used to identify significant, discontinuous price jumps in real-time. It compares the magnitude of a single return to the estimated local volatility.

    $$
    L_t = \frac{|r_t|}{\hat{\sigma}_t}
    $$

    *Where:*
    - `$|r_t|$` is the absolute price return.
    - `$\hat{\sigma}_t$` is the local volatility, typically estimated using Bipower Variation over a recent window. A jump is flagged if `$L_t$` exceeds a critical value (e.g., 3.0-3.5).

### 2. Regime Classification with HMM

The framework uses a **Gaussian Hidden Markov Model (HMM)** to classify the market's latent state into one of three regimes: **Low**, **Normal**, or **High** volatility.

*   **Viterbi Algorithm:** This dynamic programming algorithm is used to efficiently decode the most probable sequence of hidden states given the observed volatility data.
*   **Log-Transformed Features:** Volatility and jump metrics are log-transformed before being fed into the HMM. This reshapes their distributions to better fit the Gaussian assumptions of the model, leading to more accurate state classification.

### 3. Volatility Forecasting

*   **HAR-RV-J Model:** A Heterogeneous Autoregressive (HAR) model is used for forecasting next-period volatility. It captures the long-memory nature of volatility by using regressors from daily, weekly, and monthly time horizons.

    $$
    \log(\sigma_{t+1}) = \beta_0 + \beta_d \log(\text{RV}_t^d) + \beta_w \log(\text{RV}_t^w) + \beta_m \log(\text{RV}_t^m) + \beta_j \log(1 + J_t)
    $$

    *Where:*
    - `$\sigma_{t+1}$` is the volatility forecast for the next period.
    - `$\text{RV}^d, \text{RV}^w, \text{RV}^m$` are the daily, weekly, and monthly realized volatilities.
    - `$J_t$` is the realized jump component.

### 4. HFT Safety and Risk Management

*   **Hawkes Process:** Models the arrival of trades as a self-exciting point process, where each new trade increases the probability of subsequent trades. This is used to measure market intensity and detect "flash crash" type events.

    $$
    \lambda(t) = \mu + \sum_{t_i < t} \alpha e^{-\beta(t - t_i)}
    $$

    *Where:*
    - `$\lambda(t)$` is the conditional intensity at time `$t$`.
    - `$\mu$` is the baseline intensity.
    - `$\alpha$` is the magnitude of the jump in intensity after an event.
    - `$\beta$` is the rate of exponential decay of the excitement.

*   **Adaptive Risk Controls:**
    - **Dynamic Position Sizing:** Based on the HMM regime, positions are scaled up in low-volatility and scaled down in high-volatility to maintain a stable risk profile.
    - **Real-time Circuit Breakers:** Both the Hawkes intensity and Lee-Mykland jump tests are monitored. If a critical threshold is breached, execution is immediately halted or significantly curtailed.

---

## System Architecture

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

## Building the Project

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

## Usage Examples

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

## Backtesting Performance

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

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
