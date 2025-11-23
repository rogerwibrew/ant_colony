/**
 * @file LocalSearch_test.cpp
 * @brief Unit tests for the LocalSearch class
 */

#include <gtest/gtest.h>
#include "LocalSearch.h"
#include "Graph.h"
#include "City.h"
#include "Tour.h"
#include <cmath>

// Forward declaration of helper function
double calculateDistance(const std::vector<int>& sequence, const Graph& graph);

// Test fixture for LocalSearch tests
class LocalSearchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple 5-city graph for testing
        // Layout:
        //   0(0,0) -- 1(4,0) -- 2(4,3)
        //     |                    |
        //   3(0,3) ----------- 4(2,3)
        cities5 = {
            City(0, 0.0, 0.0),
            City(1, 4.0, 0.0),
            City(2, 4.0, 3.0),
            City(3, 0.0, 3.0),
            City(4, 2.0, 3.0)
        };
        graph5 = Graph(cities5);

        // Create a triangle graph (3 cities)
        citiesTriangle = {
            City(0, 0.0, 0.0),
            City(1, 4.0, 0.0),
            City(2, 2.0, 3.0)
        };
        graphTriangle = Graph(citiesTriangle);

        // Create a square graph (4 cities)
        citiesSquare = {
            City(0, 0.0, 0.0),
            City(1, 10.0, 0.0),
            City(2, 10.0, 10.0),
            City(3, 0.0, 10.0)
        };
        graphSquare = Graph(citiesSquare);
    }

    std::vector<City> cities5;
    std::vector<City> citiesTriangle;
    std::vector<City> citiesSquare;
    Graph graph5;
    Graph graphTriangle;
    Graph graphSquare;
};

// Test 2-opt on trivially small tour (should not crash)
TEST_F(LocalSearchTest, TwoOptTooSmall) {
    std::vector<int> sequence = {0, 1, 2};
    double distance = 12.0;
    Tour tour(sequence, distance);

    bool improved = LocalSearch::twoOpt(tour, graphTriangle);

    // Should return false (no improvement possible with 3 cities)
    EXPECT_FALSE(improved);
    EXPECT_EQ(tour.getSequence(), sequence);
}

// Test 2-opt on optimal tour (should not change)
TEST_F(LocalSearchTest, TwoOptAlreadyOptimal) {
    // Optimal tour for square: 0->1->2->3->0 (perimeter = 40)
    std::vector<int> sequence = {0, 1, 2, 3};
    double distance = 40.0;
    Tour tour(sequence, distance);

    bool improved = LocalSearch::twoOpt(tour, graphSquare);

    // Already optimal, no improvement
    EXPECT_FALSE(improved);
}

// Test 2-opt on suboptimal tour (should improve)
TEST_F(LocalSearchTest, TwoOptImprovesSuboptimal) {
    // Suboptimal tour for square: 0->2->1->3->0 (has crossing)
    // Distance: sqrt(200) + 10 + sqrt(200) + 10 ≈ 48.28
    std::vector<int> sequence = {0, 2, 1, 3};
    double distance = 2.0 * std::sqrt(200.0) + 20.0;
    Tour tour(sequence, distance);

    bool improved = LocalSearch::twoOpt(tour, graphSquare);

    // Should improve
    EXPECT_TRUE(improved);

    // After 2-opt, should be closer to optimal (40.0)
    EXPECT_LT(tour.getDistance(), distance);
    EXPECT_NEAR(tour.getDistance(), 40.0, 0.1);
}

// Test 2-opt systematically improves tour
TEST_F(LocalSearchTest, TwoOptMonotonicImprovement) {
    // Intentionally bad tour: 0->3->1->4->2->0
    std::vector<int> sequence = {0, 3, 1, 4, 2};
    double initialDistance = calculateDistance(sequence, graph5);
    Tour tour(sequence, initialDistance);

    bool improved = LocalSearch::twoOpt(tour, graph5);

    if (improved) {
        EXPECT_LT(tour.getDistance(), initialDistance);
    }

    // Tour should still be valid
    EXPECT_TRUE(tour.validate(5));
}

// Test 3-opt on trivially small tour
TEST_F(LocalSearchTest, ThreeOptTooSmall) {
    std::vector<int> sequence = {0, 1, 2, 3};
    double distance = 40.0;
    Tour tour(sequence, distance);

    bool improved = LocalSearch::threeOpt(tour, graphSquare);

    // Should return false (need at least 6 cities)
    EXPECT_FALSE(improved);
}

// Test 3-opt on 6-city problem
TEST_F(LocalSearchTest, ThreeOptSixCities) {
    // Create 6-city hexagon
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 2.0, 0.0),
        City(2, 3.0, 1.7),
        City(3, 2.0, 3.4),
        City(4, 0.0, 3.4),
        City(5, -1.0, 1.7)
    };
    Graph graph(cities);

    // Suboptimal tour with some crossed edges
    std::vector<int> sequence = {0, 2, 1, 4, 3, 5};
    double initialDistance = calculateDistance(sequence, graph);
    Tour tour(sequence, initialDistance);

    bool improved = LocalSearch::threeOpt(tour, graph);

    // Tour should still be valid regardless of improvement
    EXPECT_TRUE(tour.validate(6));

    if (improved) {
        EXPECT_LT(tour.getDistance(), initialDistance);
    }
}

// Test improve() method with 2-opt only
TEST_F(LocalSearchTest, ImproveTwoOptOnly) {
    // Suboptimal square tour
    std::vector<int> sequence = {0, 2, 1, 3};
    double initialDistance = 2.0 * std::sqrt(200.0) + 20.0;
    Tour tour(sequence, initialDistance);

    bool improved = LocalSearch::improve(tour, graphSquare, false);  // 2-opt only

    EXPECT_TRUE(improved);
    EXPECT_LT(tour.getDistance(), initialDistance);
}

// Test improve() method with both 2-opt and 3-opt
TEST_F(LocalSearchTest, ImproveBothMethods) {
    // Create 6-city problem
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 2.0, 0.0),
        City(2, 3.0, 1.7),
        City(3, 2.0, 3.4),
        City(4, 0.0, 3.4),
        City(5, -1.0, 1.7)
    };
    Graph graph(cities);

    // Suboptimal tour
    std::vector<int> sequence = {0, 2, 4, 1, 5, 3};
    double initialDistance = calculateDistance(sequence, graph);
    Tour tour(sequence, initialDistance);

    bool improved = LocalSearch::improve(tour, graph, true);  // Both 2-opt and 3-opt

    // Tour should still be valid
    EXPECT_TRUE(tour.validate(6));
}

// Test that 2-opt preserves tour validity
TEST_F(LocalSearchTest, TwoOptPreservesValidity) {
    std::vector<int> sequence = {0, 4, 2, 1, 3};
    double distance = calculateDistance(sequence, graph5);
    Tour tour(sequence, distance);

    LocalSearch::twoOpt(tour, graph5);

    // Tour should still be valid
    EXPECT_TRUE(tour.validate(5));

    // Should still visit all cities exactly once
    std::vector<int> newSeq = tour.getSequence();
    EXPECT_EQ(newSeq.size(), 5);

    std::vector<bool> visited(5, false);
    for (int city : newSeq) {
        EXPECT_GE(city, 0);
        EXPECT_LT(city, 5);
        EXPECT_FALSE(visited[city]);
        visited[city] = true;
    }
}

// Test that 3-opt preserves tour validity
TEST_F(LocalSearchTest, ThreeOptPreservesValidity) {
    // Create 6-city problem
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 1.0, 0.0),
        City(2, 2.0, 0.0),
        City(3, 2.0, 1.0),
        City(4, 1.0, 1.0),
        City(5, 0.0, 1.0)
    };
    Graph graph(cities);

    std::vector<int> sequence = {0, 3, 1, 4, 2, 5};
    double distance = calculateDistance(sequence, graph);
    Tour tour(sequence, distance);

    LocalSearch::threeOpt(tour, graph);

    // Tour should still be valid
    EXPECT_TRUE(tour.validate(6));

    // Should still visit all cities exactly once
    std::vector<int> newSeq = tour.getSequence();
    EXPECT_EQ(newSeq.size(), 6);

    std::vector<bool> visited(6, false);
    for (int city : newSeq) {
        EXPECT_GE(city, 0);
        EXPECT_LT(city, 6);
        EXPECT_FALSE(visited[city]);
        visited[city] = true;
    }
}

// Test 2-opt distance calculation accuracy
TEST_F(LocalSearchTest, TwoOptDistanceAccuracy) {
    std::vector<int> sequence = {0, 2, 1, 3};
    double initialDistance = 2.0 * std::sqrt(200.0) + 20.0;
    Tour tour(sequence, initialDistance);

    LocalSearch::twoOpt(tour, graphSquare);

    // Manually verify distance
    double manualDistance = 0.0;
    const std::vector<int>& newSeq = tour.getSequence();
    for (size_t i = 0; i < newSeq.size(); ++i) {
        int from = newSeq[i];
        int to = newSeq[(i + 1) % newSeq.size()];
        manualDistance += graphSquare.getDistance(from, to);
    }

    EXPECT_NEAR(tour.getDistance(), manualDistance, 1e-6);
}

// Test 3-opt distance calculation accuracy
TEST_F(LocalSearchTest, ThreeOptDistanceAccuracy) {
    // Create 6-city problem
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 2.0, 0.0),
        City(2, 3.0, 1.7),
        City(3, 2.0, 3.4),
        City(4, 0.0, 3.4),
        City(5, -1.0, 1.7)
    };
    Graph graph(cities);

    std::vector<int> sequence = {0, 2, 1, 4, 3, 5};
    double initialDistance = calculateDistance(sequence, graph);
    Tour tour(sequence, initialDistance);

    LocalSearch::threeOpt(tour, graph);

    // Manually verify distance
    double manualDistance = 0.0;
    const std::vector<int>& newSeq = tour.getSequence();
    for (size_t i = 0; i < newSeq.size(); ++i) {
        int from = newSeq[i];
        int to = newSeq[(i + 1) % newSeq.size()];
        manualDistance += graph.getDistance(from, to);
    }

    EXPECT_NEAR(tour.getDistance(), manualDistance, 1e-6);
}

// Helper function to calculate tour distance
double calculateDistance(const std::vector<int>& sequence, const Graph& graph) {
    double distance = 0.0;
    for (size_t i = 0; i < sequence.size(); ++i) {
        int from = sequence[i];
        int to = sequence[(i + 1) % sequence.size()];
        distance += graph.getDistance(from, to);
    }
    return distance;
}

// Test that repeated 2-opt converges (doesn't loop forever)
TEST_F(LocalSearchTest, TwoOptConverges) {
    std::vector<int> sequence = {0, 4, 2, 1, 3};
    double distance = calculateDistance(sequence, graph5);
    Tour tour(sequence, distance);

    int iterations = 0;
    const int maxIterations = 100;

    while (LocalSearch::twoOpt(tour, graph5) && iterations < maxIterations) {
        iterations++;
    }

    // Should converge well before max iterations
    EXPECT_LT(iterations, maxIterations);
    EXPECT_TRUE(tour.validate(5));
}

// Test that repeated 3-opt converges
TEST_F(LocalSearchTest, ThreeOptConverges) {
    // Create 6-city problem
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 2.0, 0.0),
        City(2, 3.0, 1.7),
        City(3, 2.0, 3.4),
        City(4, 0.0, 3.4),
        City(5, -1.0, 1.7)
    };
    Graph graph(cities);

    std::vector<int> sequence = {0, 2, 4, 1, 5, 3};
    double distance = calculateDistance(sequence, graph);
    Tour tour(sequence, distance);

    int iterations = 0;
    const int maxIterations = 100;

    while (LocalSearch::threeOpt(tour, graph) && iterations < maxIterations) {
        iterations++;
    }

    // Should converge well before max iterations
    EXPECT_LT(iterations, maxIterations);
    EXPECT_TRUE(tour.validate(6));
}
