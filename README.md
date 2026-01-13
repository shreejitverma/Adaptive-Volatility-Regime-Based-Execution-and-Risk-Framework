# Adaptive Volatility Regime-Based Execution & Risk Framework

## 1. Executive Summary

This project presents a high-performance C++ framework for **adaptive trading and risk management**. The core thesis is that financial market volatility is not constant but switches between distinct **regimes** (e.g., low, normal, and high). By identifying these regimes in real-time, we can dynamically adjust execution strategies and risk parameters to optimize performance and mitigate catastrophic losses.

The framework is designed for high-frequency applications and integrates several key quantitative finance concepts:
- **Microstructure-Robust Volatility Estimation:** Using advanced estimators like TSRV and MedRV to handle market noise and jumps.
- **Regime Detection:** Employing a Hidden Markov Model (HMM) to classify the current market state.
- **Real-Time Safety Nets:** Utilizing Hawkes Processes for flash crash detection and Lee-Mykland tests for instantaneous price jump filtering.
- **Adaptive Execution:** Adjusting position sizes and execution algorithms based on the detected regime.

This document outlines the theoretical underpinnings, system architecture, implementation details, and backtesting results of the framework.

---

## 2. Theoretical Framework

The model is built upon a cascade of econometric and statistical techniques, each addressing a specific market phenomenon.

### 2.1. Volatility and Jump Measurement

Standard Realized Volatility (`RV`) is susceptible to microstructure noise and jumps. We use more advanced estimators.

*   **Median Realized Volatility (MedRV):**
    To obtain a continuous volatility signal that is robust to sporadic price jumps, we use MedRV. It replaces the squared return in a standard `RV` calculation with the median of a local three-return window.

    $$ 
    \text{MedRV}_t = \frac{\pi}{6-4\sqrt{3}+\pi} \frac{N}{N-2} \sum_{i=2}^{N} \text{med}(|r_i|, |r_{i-1}|, |r_{i-2}|)^2 
    $$ 

    This effectively filters out large, isolated returns, providing a cleaner input for our regime detection model.

*   **Lee-Mykland Jump Test:**
    To explicitly identify and react to jumps, we use the statistical test proposed by Lee and Mykland (2008). It standardizes each return by the local volatility.

    $$ 
    L_t = \frac{|r_t|}{\hat{\sigma}_t} 
    $$ 
    
A value of `$L_t$` exceeding a critical threshold (e.g., 3.0) provides statistical evidence of a jump at time `$t$`, allowing our system to enter a defensive state.

### 2.2. Regime Modeling (Hidden Markov Model)

We model the market as a system with unobservable (hidden) states that drive observable phenomena (volatility, jumps).

*   **Model:** A 3-state Gaussian HMM is used to represent **Low**, **Normal**, and **High** volatility regimes. Each state is defined by a unique Gaussian distribution over our input features (e.g., log-volatility and log-jumps).
*   **Decoding:** The **Viterbi algorithm** is applied to find the most likely sequence of regimes given the history of observations, providing a real-time indicator of the current market state.

### 2.3. Market Activity Modeling (Hawkes Process)

To monitor for HFT-specific risks like order book cascades or flash crashes, we model the arrival of trades using a self-exciting Hawkes Process.

*   **Conditional Intensity:** The probability of a trade occurring at time `$t$` is given by the conditional intensity `$\\lambda(t)$`.

    $$ 
    \lambda(t) = \mu + \sum_{t_i < t} \alpha e^{-\beta(t - t_i)} 
    $$ 

    *Where:*
    - `$\\mu$` is the baseline arrival rate.
    - Each past event `$t_i$` adds `$\\alpha$` to the intensity, which then decays exponentially at a rate of `$\\beta$`.
    
A sudden spike in `$\\lambda(t)$` indicates a market frenzy, which triggers our circuit breaker logic.

---

## 3. System Architecture and Implementation

The framework is a modular C++ library (`AdaptiveVolCore`) with a demonstration executable (`AdaptiveVolDemo`).

*   **Performance:** Code is heavily optimized using C++17, Eigen for vectorized linear algebra, and aggressive compiler flags (`-O3 -march=native`).
*   **Dependencies:** `Eigen` and `GoogleTest` are managed automatically via `CMake` and `FetchContent`.
*   **Modularity:**
    - `VolatilityEstimators`: Implements all volatility/jump metrics.
    - `HMMRegimeDetector`: Implements the Viterbi algorithm for state decoding.
    - `HawkesModel`: Implements the intensity tracking model.
    - `RiskManager`: Contains logic for adaptive position sizing.
    - `BacktestEngine`: A simple, event-driven engine to simulate strategy performance.
    - `ReportGenerator`: Prints performance summaries.

---

## 4. Findings and Backtest Performance

To validate the framework, we simulate a simple adaptive trend-following strategy on synthetic data designed to exhibit regime changes, drift, and jumps.

*   **Strategy:** A simple SMA crossover (5-day vs. 20-day) is used to generate directional signals.
*   **Adaptation Logic:**
    1.  **Regime Sizing:** Position size is multiplied by 1.5x in low-volatility regimes and 0.5x in high-volatility regimes.
    2.  **Jump Filtering:** If the Lee-Mykland statistic exceeds 3.0, position size is reduced by 80% for that period.
    3.  **Intensity Halts:** If Hawkes intensity exceeds a critical threshold (simulated), trading is halted.

The results demonstrate the value of adaptation. A static version of this strategy would suffer significant drawdowns during high-volatility periods, whereas our adaptive approach successfully mitigates risk.

### Simulated Performance Report

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

*Note: This performance is on synthetic data tailored to validate the model's adaptive capabilities and is not indicative of live trading results.*

---

## 5. Building and Usage

### Prerequisites
- **CMake** (3.14+)
- **C++ Compiler** (GCC, Clang, MSVC) supporting C++17

### Build Steps
```bash
git clone https://github.com/shreejitverma/Adaptive-Volatility-Regime-Based-Execution-and-Risk-Framework.git
cd Adaptive-Volatility-Regime-Based-Execution-and-Risk-Framework
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

### Running Tests and Demo
```bash
# Run unit tests
./UnitTests

# Run the backtest simulation
./AdaptiveVolDemo
```