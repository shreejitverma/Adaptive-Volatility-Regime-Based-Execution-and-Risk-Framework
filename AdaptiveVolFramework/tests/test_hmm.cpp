#include <gtest/gtest.h>
#include "../include/adaptive_exec/HMMRegimeDetector.hpp"

using namespace AdaptiveExec;

TEST(HMMTest, ViterbiDecoding) {
    HMMRegimeDetector hmm(2); // 2 states: Low, High
    
    Vector start(2); start << 0.5, 0.5;
    Matrix trans(2,2); trans << 0.9, 0.1, 0.1, 0.9;
    Matrix means(2,1); means << 0.0, 10.0; // State 0 expects 0, State 1 expects 10
    Matrix vars(2,1); vars << 1.0, 1.0;
    
    hmm.setParameters(start, trans, means, vars);
    
    Matrix obs(4,1);
    obs << 0.1, 0.2, 9.9, 10.1; 
    // Should be State 0, 0, 1, 1
    
    std::vector<int> states = hmm.predictStates(obs);
    
    ASSERT_EQ(states.size(), 4);
    EXPECT_EQ(states[0], 0);
    EXPECT_EQ(states[1], 0);
    EXPECT_EQ(states[2], 1);
    EXPECT_EQ(states[3], 1);
}
