#include <gtest/gtest.h>
#include "AntColony.h"
#include "Graph.h"
#include "City.h"

// Helper function to create a simple triangle graph
Graph createTriangleGraph() {
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 3.0, 0.0),
        City(2, 0.0, 4.0)
    };
    return Graph(cities);
}

// Helper function to create a square graph
Graph createSquareGraph() {
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 1.0, 0.0),
        City(2, 1.0, 1.0),
        City(3, 0.0, 1.0)
    };
    return Graph(cities);
}

// Test constructor
TEST(AntColonyTest, Constructor) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    EXPECT_EQ(colony.getNumAnts(), 10);
    EXPECT_DOUBLE_EQ(colony.getAlpha(), 1.0);
    EXPECT_DOUBLE_EQ(colony.getBeta(), 2.0);
    EXPECT_DOUBLE_EQ(colony.getRho(), 0.5);
    EXPECT_DOUBLE_EQ(colony.getQ(), 100.0);
}

// Test initialize
TEST(AntColonyTest, Initialize) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 5, 1.0, 2.0, 0.5, 100.0);

    colony.initialize();

    EXPECT_EQ(colony.getConvergenceData().size(), 0);
}

// Test constructSolutions
TEST(AntColonyTest, ConstructSolutions) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.initialize();
    colony.constructSolutions();

    // All ants should have constructed complete tours
    // We can't directly access ants, but we can run an iteration and check convergence data
}

// Test runIteration
TEST(AntColonyTest, RunIteration) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.initialize();
    colony.runIteration();

    // Should have one iteration recorded
    EXPECT_EQ(colony.getConvergenceData().size(), 1);

    // Best tour distance should be > 0
    EXPECT_GT(colony.getBestTour().getDistance(), 0.0);
}

// Test multiple iterations
TEST(AntColonyTest, MultipleIterations) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.initialize();
    colony.runIteration();
    colony.runIteration();
    colony.runIteration();

    EXPECT_EQ(colony.getConvergenceData().size(), 3);
}

// Test solve method
TEST(AntColonyTest, Solve) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(10);

    EXPECT_EQ(colony.getConvergenceData().size(), 10);
    EXPECT_GT(bestTour.getDistance(), 0.0);
    EXPECT_EQ(bestTour.getSequence().size(), 3);

    // Verify tour is valid
    EXPECT_TRUE(bestTour.validate(3));
}

// Test convergence - best tour should improve or stay same
TEST(AntColonyTest, Convergence) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(20);

    const auto& convergenceData = colony.getConvergenceData();
    EXPECT_EQ(convergenceData.size(), 20);

    // Best tour from solve should match the best from convergence data
    double minConvergence = *std::min_element(convergenceData.begin(), convergenceData.end());
    EXPECT_DOUBLE_EQ(bestTour.getDistance(), minConvergence);
}

// Test that best tour doesn't get worse over iterations
TEST(AntColonyTest, BestTourMonotonic) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 15, 1.0, 2.0, 0.5, 100.0);

    colony.initialize();

    double previousBest = std::numeric_limits<double>::max();

    for (int i = 0; i < 10; ++i) {
        colony.runIteration();
        double currentBest = colony.getBestTour().getDistance();

        // Best should never get worse
        EXPECT_LE(currentBest, previousBest);
        previousBest = currentBest;
    }
}

// Test small problem - triangle (known optimal is perimeter)
TEST(AntColonyTest, TriangleProblem) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(50);

    // Triangle perimeter: 3 + 4 + 5 = 12.0
    // ACO should find this or very close to it
    EXPECT_NEAR(bestTour.getDistance(), 12.0, 0.1);
}

// Test single city
TEST(AntColonyTest, SingleCity) {
    std::vector<City> cities = { City(0, 0.0, 0.0) };
    Graph graph(cities);
    AntColony colony(graph, 5, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(5);

    EXPECT_EQ(bestTour.getSequence().size(), 1);
    EXPECT_DOUBLE_EQ(bestTour.getDistance(), 0.0);
}

// Test two cities
TEST(AntColonyTest, TwoCities) {
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 5.0, 0.0)
    };
    Graph graph(cities);
    AntColony colony(graph, 5, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(5);

    EXPECT_EQ(bestTour.getSequence().size(), 2);
    // Distance should be 5 + 5 = 10
    EXPECT_DOUBLE_EQ(bestTour.getDistance(), 10.0);
}

// Test different alpha values
TEST(AntColonyTest, DifferentAlpha) {
    Graph graph = createTriangleGraph();

    // Alpha = 0 (ignore pheromone)
    AntColony colony1(graph, 10, 0.0, 2.0, 0.5, 100.0);
    Tour tour1 = colony1.solve(10);
    EXPECT_GT(tour1.getDistance(), 0.0);

    // Alpha = 2 (high pheromone importance)
    AntColony colony2(graph, 10, 2.0, 2.0, 0.5, 100.0);
    Tour tour2 = colony2.solve(10);
    EXPECT_GT(tour2.getDistance(), 0.0);
}

// Test different beta values
TEST(AntColonyTest, DifferentBeta) {
    Graph graph = createTriangleGraph();

    // Beta = 0 (ignore distance)
    AntColony colony1(graph, 10, 1.0, 0.0, 0.5, 100.0);
    Tour tour1 = colony1.solve(10);
    EXPECT_GT(tour1.getDistance(), 0.0);

    // Beta = 5 (high distance importance)
    AntColony colony2(graph, 10, 1.0, 5.0, 0.5, 100.0);
    Tour tour2 = colony2.solve(10);
    EXPECT_GT(tour2.getDistance(), 0.0);
}

// Test different evaporation rates
TEST(AntColonyTest, DifferentEvaporationRates) {
    Graph graph = createSquareGraph();

    // Low evaporation (rho = 0.1)
    AntColony colony1(graph, 10, 1.0, 2.0, 0.1, 100.0);
    Tour tour1 = colony1.solve(10);
    EXPECT_GT(tour1.getDistance(), 0.0);

    // High evaporation (rho = 0.9)
    AntColony colony2(graph, 10, 1.0, 2.0, 0.9, 100.0);
    Tour tour2 = colony2.solve(10);
    EXPECT_GT(tour2.getDistance(), 0.0);
}

// Test different Q values
TEST(AntColonyTest, DifferentQValues) {
    Graph graph = createSquareGraph();

    // Small Q
    AntColony colony1(graph, 10, 1.0, 2.0, 0.5, 10.0);
    Tour tour1 = colony1.solve(10);
    EXPECT_GT(tour1.getDistance(), 0.0);

    // Large Q
    AntColony colony2(graph, 10, 1.0, 2.0, 0.5, 1000.0);
    Tour tour2 = colony2.solve(10);
    EXPECT_GT(tour2.getDistance(), 0.0);
}

// Test different number of ants
TEST(AntColonyTest, DifferentNumAnts) {
    Graph graph = createSquareGraph();

    // Few ants
    AntColony colony1(graph, 3, 1.0, 2.0, 0.5, 100.0);
    Tour tour1 = colony1.solve(10);
    EXPECT_GT(tour1.getDistance(), 0.0);

    // Many ants
    AntColony colony2(graph, 50, 1.0, 2.0, 0.5, 100.0);
    Tour tour2 = colony2.solve(10);
    EXPECT_GT(tour2.getDistance(), 0.0);
}

// Test that solve returns best tour
TEST(AntColonyTest, SolveReturnsBestTour) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 15, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(10);

    // Best tour from solve should match getBestTour
    EXPECT_DOUBLE_EQ(bestTour.getDistance(), colony.getBestTour().getDistance());
    EXPECT_EQ(bestTour.getSequence(), colony.getBestTour().getSequence());
}

// Test convergence data length
TEST(AntColonyTest, ConvergenceDataLength) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.solve(25);

    EXPECT_EQ(colony.getConvergenceData().size(), 25);
}

// Test that all convergence values are positive
TEST(AntColonyTest, ConvergenceDataPositive) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.solve(15);

    for (double distance : colony.getConvergenceData()) {
        EXPECT_GT(distance, 0.0);
    }
}

// Test square problem (optimal tour should be perimeter)
TEST(AntColonyTest, SquareProblem) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 30, 1.0, 2.0, 0.5, 100.0);

    Tour bestTour = colony.solve(100);

    // Square perimeter: 1 + 1 + 1 + 1 = 4.0
    // ACO should find this or very close to it
    EXPECT_NEAR(bestTour.getDistance(), 4.0, 0.1);
}

// Test that multiple solve calls reset properly
TEST(AntColonyTest, MultipleSolveCalls) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    Tour tour1 = colony.solve(5);
    EXPECT_EQ(colony.getConvergenceData().size(), 5);

    Tour tour2 = colony.solve(10);
    EXPECT_EQ(colony.getConvergenceData().size(), 10);  // Should reset
}

// Test parameter getters
TEST(AntColonyTest, ParameterGetters) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 15, 1.5, 2.5, 0.6, 150.0);

    EXPECT_EQ(colony.getNumAnts(), 15);
    EXPECT_DOUBLE_EQ(colony.getAlpha(), 1.5);
    EXPECT_DOUBLE_EQ(colony.getBeta(), 2.5);
    EXPECT_DOUBLE_EQ(colony.getRho(), 0.6);
    EXPECT_DOUBLE_EQ(colony.getQ(), 150.0);
}

// Test that solution improves over iterations (on average)
TEST(AntColonyTest, SolutionImprovement) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    colony.solve(50);

    const auto& convergenceData = colony.getConvergenceData();

    // Average of first 10 iterations
    double earlyAvg = 0.0;
    for (int i = 0; i < 10; ++i) {
        earlyAvg += convergenceData[i];
    }
    earlyAvg /= 10.0;

    // Average of last 10 iterations
    double lateAvg = 0.0;
    for (int i = 40; i < 50; ++i) {
        lateAvg += convergenceData[i];
    }
    lateAvg /= 10.0;

    // Later iterations should generally have better (lower) distances
    EXPECT_LE(lateAvg, earlyAvg);
}

// ============================================================================
// Threading Tests (OpenMP)
// ============================================================================

// Test threading control methods exist and can be called
TEST(AntColonyTest, ThreadingControlMethods) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    // These methods should not crash
    colony.setUseParallel(true);
    colony.setUseParallel(false);
    colony.setNumThreads(0);  // Auto
    colony.setNumThreads(1);  // Serial
    colony.setNumThreads(4);  // Specific count

    SUCCEED();  // If we got here, methods work
}

// Test serial execution (explicitly disabled parallel)
TEST(AntColonyTest, SerialExecution) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setUseParallel(false);
    colony.setNumThreads(1);

    Tour bestTour = colony.solve(10);

    EXPECT_GT(bestTour.getDistance(), 0.0);
    EXPECT_EQ(bestTour.getSequence().size(), 4);
    EXPECT_TRUE(bestTour.validate(4));
}

// Test parallel execution (explicitly enabled)
TEST(AntColonyTest, ParallelExecution) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    colony.setUseParallel(true);
    colony.setNumThreads(4);

    Tour bestTour = colony.solve(10);

    EXPECT_GT(bestTour.getDistance(), 0.0);
    EXPECT_EQ(bestTour.getSequence().size(), 4);
    EXPECT_TRUE(bestTour.validate(4));
}

// Test that both serial and parallel produce valid results
TEST(AntColonyTest, SerialParallelBothValid) {
    Graph graph = createTriangleGraph();

    // Serial run
    AntColony serialColony(graph, 15, 1.0, 2.0, 0.5, 100.0);
    serialColony.setUseParallel(false);
    Tour serialTour = serialColony.solve(20);

    // Parallel run
    AntColony parallelColony(graph, 15, 1.0, 2.0, 0.5, 100.0);
    parallelColony.setUseParallel(true);
    parallelColony.setNumThreads(4);
    Tour parallelTour = parallelColony.solve(20);

    // Both should produce valid tours
    EXPECT_TRUE(serialTour.validate(3));
    EXPECT_TRUE(parallelTour.validate(3));

    // Both should find reasonable solutions (within 2× of each other)
    double ratio = serialTour.getDistance() / parallelTour.getDistance();
    EXPECT_GT(ratio, 0.5);  // Neither should be more than 2× worse
    EXPECT_LT(ratio, 2.0);
}

// Test different thread counts all work
TEST(AntColonyTest, VariousThreadCounts) {
    Graph graph = createSquareGraph();

    // Test with 1, 2, 4 threads
    for (int numThreads : {1, 2, 4}) {
        AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);
        colony.setNumThreads(numThreads);

        Tour bestTour = colony.solve(5);

        EXPECT_GT(bestTour.getDistance(), 0.0) << "Failed with " << numThreads << " threads";
        EXPECT_TRUE(bestTour.validate(4)) << "Invalid tour with " << numThreads << " threads";
    }
}

// Test auto thread detection (numThreads = 0)
TEST(AntColonyTest, AutoThreadDetection) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setNumThreads(0);  // Auto-detect

    Tour bestTour = colony.solve(10);

    EXPECT_TRUE(bestTour.validate(3));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// ==================== Elitist Strategy Tests ====================

// Test basic elitist strategy enables correctly
TEST(AntColonyTest, ElitistStrategyBasic) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setUseElitist(true);
    Tour bestTour = colony.solve(20);

    // Should produce valid solution
    EXPECT_TRUE(bestTour.validate(4));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test elitist vs non-elitist on same problem
TEST(AntColonyTest, ElitistVsNonElitist) {
    Graph graph = createSquareGraph();

    // Non-elitist run
    AntColony normalColony(graph, 20, 1.0, 2.0, 0.5, 100.0);
    normalColony.setUseElitist(false);
    Tour normalTour = normalColony.solve(50);

    // Elitist run
    AntColony elitistColony(graph, 20, 1.0, 2.0, 0.5, 100.0);
    elitistColony.setUseElitist(true);
    Tour elitistTour = elitistColony.solve(50);

    // Both should be valid
    EXPECT_TRUE(normalTour.validate(4));
    EXPECT_TRUE(elitistTour.validate(4));

    // Elitist should typically find better or equal solutions (allow some variance)
    // At minimum, should be within reasonable bounds
    double ratio = elitistTour.getDistance() / normalTour.getDistance();
    EXPECT_GT(ratio, 0.5);  // Not more than 2× worse
    EXPECT_LT(ratio, 2.0);  // Reasonable quality
}

// Test custom elitist weight
TEST(AntColonyTest, ElitistCustomWeight) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setUseElitist(true);
    colony.setElitistWeight(50.0);  // Custom weight instead of numAnts

    Tour bestTour = colony.solve(15);

    EXPECT_TRUE(bestTour.validate(3));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test pheromone mode: "best-iteration"
TEST(AntColonyTest, PheromoneModeIterationBest) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 15, 1.0, 2.0, 0.5, 100.0);

    colony.setPheromoneMode("best-iteration");
    Tour bestTour = colony.solve(20);

    EXPECT_TRUE(bestTour.validate(4));
    EXPECT_GT(bestTour.getDistance(), 0.0);
    EXPECT_EQ(colony.getConvergenceData().size(), 20);
}

// Test pheromone mode: "best-so-far"
TEST(AntColonyTest, PheromoneModeBestSoFar) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 15, 1.0, 2.0, 0.5, 100.0);

    colony.setPheromoneMode("best-so-far");
    Tour bestTour = colony.solve(20);

    EXPECT_TRUE(bestTour.validate(4));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test pheromone mode: "rank"
TEST(AntColonyTest, PheromoneModeRank) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    colony.setPheromoneMode("rank");
    colony.setRankSize(5);  // Only top 5 ants deposit
    Tour bestTour = colony.solve(15);

    EXPECT_TRUE(bestTour.validate(4));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test rank mode with auto rank size
TEST(AntColonyTest, RankModeAutoSize) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setPheromoneMode("rank");
    colony.setRankSize(0);  // Auto = numAnts / 2
    Tour bestTour = colony.solve(10);

    EXPECT_TRUE(bestTour.validate(3));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test combining elitist with different pheromone modes
TEST(AntColonyTest, ElitistWithRankMode) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 20, 1.0, 2.0, 0.5, 100.0);

    colony.setUseElitist(true);
    colony.setPheromoneMode("rank");
    colony.setRankSize(10);
    colony.setElitistWeight(15.0);

    Tour bestTour = colony.solve(25);

    EXPECT_TRUE(bestTour.validate(4));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test elitist with best-iteration mode
TEST(AntColonyTest, ElitistWithBestIterationMode) {
    Graph graph = createTriangleGraph();
    AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);

    colony.setUseElitist(true);
    colony.setPheromoneMode("best-iteration");

    Tour bestTour = colony.solve(20);

    EXPECT_TRUE(bestTour.validate(3));
    EXPECT_GT(bestTour.getDistance(), 0.0);
}

// Test all pheromone modes produce valid tours
TEST(AntColonyTest, AllPheromoneModes) {
    Graph graph = createSquareGraph();
    std::vector<std::string> modes = {"all", "best-iteration", "best-so-far", "rank"};

    for (const auto& mode : modes) {
        AntColony colony(graph, 10, 1.0, 2.0, 0.5, 100.0);
        colony.setPheromoneMode(mode);

        Tour bestTour = colony.solve(10);

        EXPECT_TRUE(bestTour.validate(4)) << "Failed with mode: " << mode;
        EXPECT_GT(bestTour.getDistance(), 0.0) << "Failed with mode: " << mode;
    }
}

// Test convergence behavior with elitist strategy
TEST(AntColonyTest, ElitistConvergence) {
    Graph graph = createSquareGraph();
    AntColony colony(graph, 15, 1.0, 2.0, 0.5, 100.0);

    colony.setUseElitist(true);
    colony.solve(30);

    const auto& convergence = colony.getConvergenceData();
    EXPECT_EQ(convergence.size(), 30);

    // Check monotonic improvement (best should never get worse)
    double bestSoFar = std::numeric_limits<double>::max();
    for (double dist : convergence) {
        EXPECT_LE(dist, bestSoFar);
        bestSoFar = std::min(bestSoFar, dist);
    }
}
