#include <gtest/gtest.h>
#include "PheromoneMatrix.h"
#include <limits>

// Test constructor and initialization
TEST(PheromoneMatrixTest, ConstructorInitialization) {
    PheromoneMatrix matrix(5, 1.0);
    EXPECT_EQ(matrix.getNumCities(), 5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 3), 1.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(4, 1), 1.0);
}

// Test initialize method
TEST(PheromoneMatrixTest, InitializeMethod) {
    PheromoneMatrix matrix(4, 1.0);
    matrix.initialize(2.5);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_DOUBLE_EQ(matrix.getPheromone(i, j), 2.5);
        }
    }
}

// Test getPheromone and setPheromone
TEST(PheromoneMatrixTest, GetSetPheromone) {
    PheromoneMatrix matrix(3, 1.0);
    matrix.setPheromone(0, 1, 5.0);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 5.0);
    // Should be symmetric for undirected TSP
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 0), 5.0);
}

// Test symmetry after setPheromone
TEST(PheromoneMatrixTest, SetPheromoneSymmetry) {
    PheromoneMatrix matrix(4, 1.0);
    matrix.setPheromone(1, 3, 7.5);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 3), 7.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(3, 1), 7.5);

    // Other values should remain unchanged
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 1.0);
}

// Test evaporation with rho = 0.5
TEST(PheromoneMatrixTest, EvaporationHalfRate) {
    PheromoneMatrix matrix(3, 10.0);
    matrix.evaporate(0.5);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ(matrix.getPheromone(i, j), 5.0);
        }
    }
}

// Test evaporation with rho = 0.2
TEST(PheromoneMatrixTest, EvaporationPartialRate) {
    PheromoneMatrix matrix(2, 100.0);
    matrix.evaporate(0.2);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 0), 80.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 80.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 0), 80.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 1), 80.0);
}

// Test multiple evaporation cycles
TEST(PheromoneMatrixTest, MultipleEvaporations) {
    PheromoneMatrix matrix(2, 100.0);
    matrix.evaporate(0.5);
    matrix.evaporate(0.5);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 25.0);
}

// Test depositPheromone
TEST(PheromoneMatrixTest, DepositPheromone) {
    PheromoneMatrix matrix(3, 1.0);
    matrix.depositPheromone(0, 2, 3.0);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 4.0);
    // Should be symmetric
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 0), 4.0);

    // Other edges should remain unchanged
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 1.0);
}

// Test multiple deposits
TEST(PheromoneMatrixTest, MultipleDeposits) {
    PheromoneMatrix matrix(3, 1.0);
    matrix.depositPheromone(1, 2, 2.0);
    matrix.depositPheromone(1, 2, 3.0);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 2), 6.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 1), 6.0);
}

// Test evaporation followed by deposit
TEST(PheromoneMatrixTest, EvaporationThenDeposit) {
    PheromoneMatrix matrix(3, 10.0);
    matrix.evaporate(0.5); // 10.0 * 0.5 = 5.0
    matrix.depositPheromone(0, 1, 3.0); // 5.0 + 3.0 = 8.0

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 8.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 0), 8.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 5.0);
}

// Test clampPheromones with default bounds
TEST(PheromoneMatrixTest, ClampPheromonesDefault) {
    PheromoneMatrix matrix(2, 1.0);
    matrix.setPheromone(0, 1, 100.0);
    matrix.clampPheromones();

    // With default bounds (0.0 to max), should not change
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 100.0);
}

// Test clampPheromones with custom minimum
TEST(PheromoneMatrixTest, ClampPheromonesMinimum) {
    PheromoneMatrix matrix(3, 1.0);
    matrix.setMinPheromone(2.0);
    matrix.setPheromone(0, 1, 0.5);
    matrix.clampPheromones();

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 0), 2.0);
}

// Test clampPheromones with custom maximum
TEST(PheromoneMatrixTest, ClampPheromonesMaximum) {
    PheromoneMatrix matrix(3, 1.0);
    matrix.setMaxPheromone(5.0);
    matrix.setPheromone(0, 2, 10.0);
    matrix.clampPheromones();

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 5.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 0), 5.0);
}

// Test clampPheromones with both bounds
TEST(PheromoneMatrixTest, ClampPheromonesBothBounds) {
    PheromoneMatrix matrix(4, 5.0);
    matrix.setMinPheromone(2.0);
    matrix.setMaxPheromone(8.0);

    matrix.setPheromone(0, 1, 1.0);  // Below min
    matrix.setPheromone(1, 2, 5.0);  // Within range
    matrix.setPheromone(2, 3, 10.0); // Above max

    matrix.clampPheromones();

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 2), 5.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 3), 8.0);
}

// Test getters for bounds
TEST(PheromoneMatrixTest, GetBounds) {
    PheromoneMatrix matrix(3, 1.0);

    EXPECT_DOUBLE_EQ(matrix.getMinPheromone(), 0.0);
    EXPECT_DOUBLE_EQ(matrix.getMaxPheromone(), std::numeric_limits<double>::max());

    matrix.setMinPheromone(1.5);
    matrix.setMaxPheromone(10.0);

    EXPECT_DOUBLE_EQ(matrix.getMinPheromone(), 1.5);
    EXPECT_DOUBLE_EQ(matrix.getMaxPheromone(), 10.0);
}

// Test single city (edge case)
TEST(PheromoneMatrixTest, SingleCity) {
    PheromoneMatrix matrix(1, 1.0);

    EXPECT_EQ(matrix.getNumCities(), 1);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 0), 1.0);

    matrix.evaporate(0.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 0), 0.5);

    matrix.depositPheromone(0, 0, 1.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 0), 2.0);
}

// Test zero initial pheromone
TEST(PheromoneMatrixTest, ZeroInitialPheromone) {
    PheromoneMatrix matrix(3, 0.0);

    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 0.0);

    matrix.depositPheromone(0, 1, 5.0);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 5.0);
}

// Test complete evaporation (rho = 1.0)
TEST(PheromoneMatrixTest, CompleteEvaporation) {
    PheromoneMatrix matrix(3, 10.0);
    matrix.evaporate(1.0);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ(matrix.getPheromone(i, j), 0.0);
        }
    }
}

// Test no evaporation (rho = 0.0)
TEST(PheromoneMatrixTest, NoEvaporation) {
    PheromoneMatrix matrix(3, 10.0);
    matrix.evaporate(0.0);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_DOUBLE_EQ(matrix.getPheromone(i, j), 10.0);
        }
    }
}

// Test realistic ACO scenario
TEST(PheromoneMatrixTest, RealisticACOScenario) {
    PheromoneMatrix matrix(5, 1.0);
    matrix.setMinPheromone(0.1);
    matrix.setMaxPheromone(10.0);

    // Simulate one iteration
    matrix.evaporate(0.5); // Evaporate with rho = 0.5

    // Ant deposits on a tour: 0 -> 1 -> 2 -> 3 -> 0
    double deposit = 2.0;
    matrix.depositPheromone(0, 1, deposit);
    matrix.depositPheromone(1, 2, deposit);
    matrix.depositPheromone(2, 3, deposit);
    matrix.depositPheromone(3, 0, deposit);

    matrix.clampPheromones();

    // Edges on the tour should have increased pheromone
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 1), 2.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(1, 2), 2.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(2, 3), 2.5);
    EXPECT_DOUBLE_EQ(matrix.getPheromone(3, 0), 2.5);

    // Edges not on the tour should have lower pheromone
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 0.5);

    // But clamping should prevent it from going too low
    matrix.evaporate(0.99);
    matrix.clampPheromones();
    EXPECT_DOUBLE_EQ(matrix.getPheromone(0, 2), 0.1); // Clamped to min
}
