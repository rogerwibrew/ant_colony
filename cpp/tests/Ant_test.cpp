#include <gtest/gtest.h>
#include "Ant.h"
#include "Graph.h"
#include "City.h"
#include "PheromoneMatrix.h"
#include <set>

// Helper function to create a simple 3-city graph
Graph createSimpleGraph() {
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 3.0, 0.0),
        City(2, 0.0, 4.0)
    };
    return Graph(cities);
}

// Test constructor
TEST(AntTest, Constructor) {
    Ant ant(0, 5);

    EXPECT_EQ(ant.getCurrentCity(), 0);
    EXPECT_EQ(ant.getTour().size(), 1);
    EXPECT_EQ(ant.getTour()[0], 0);
    EXPECT_TRUE(ant.hasVisited(0));
    EXPECT_FALSE(ant.hasVisited(1));
    EXPECT_DOUBLE_EQ(ant.getTourLength(), 0.0);
    EXPECT_FALSE(ant.hasVisitedAll());
}

// Test reset
TEST(AntTest, Reset) {
    Ant ant(0, 3);
    Graph graph = createSimpleGraph();

    // Visit a city
    ant.visitCity(1, graph);

    EXPECT_EQ(ant.getTour().size(), 2);
    EXPECT_TRUE(ant.hasVisited(1));

    // Reset
    ant.reset(2);

    EXPECT_EQ(ant.getCurrentCity(), 2);
    EXPECT_EQ(ant.getTour().size(), 1);
    EXPECT_EQ(ant.getTour()[0], 2);
    EXPECT_TRUE(ant.hasVisited(2));
    EXPECT_FALSE(ant.hasVisited(0));
    EXPECT_FALSE(ant.hasVisited(1));
    EXPECT_DOUBLE_EQ(ant.getTourLength(), 0.0);
}

// Test visitCity
TEST(AntTest, VisitCity) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);

    EXPECT_EQ(ant.getCurrentCity(), 1);
    EXPECT_TRUE(ant.hasVisited(1));
    EXPECT_EQ(ant.getTour().size(), 2);
    EXPECT_EQ(ant.getTour()[1], 1);

    // Distance from (0,0) to (3,0) is 3.0
    EXPECT_DOUBLE_EQ(ant.getTourLength(), 3.0);
}

// Test visitCity multiple times
TEST(AntTest, VisitMultipleCities) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);
    ant.visitCity(2, graph);

    EXPECT_EQ(ant.getCurrentCity(), 2);
    EXPECT_EQ(ant.getTour().size(), 3);
    EXPECT_EQ(ant.getTour()[0], 0);
    EXPECT_EQ(ant.getTour()[1], 1);
    EXPECT_EQ(ant.getTour()[2], 2);

    // Distance: (0,0)->(3,0) = 3.0, (3,0)->(0,4) = 5.0
    EXPECT_DOUBLE_EQ(ant.getTourLength(), 8.0);
}

// Test visitCity with already visited city throws exception
TEST(AntTest, VisitAlreadyVisitedCity) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    EXPECT_THROW(ant.visitCity(0, graph), std::runtime_error);
}

// Test hasVisitedAll
TEST(AntTest, HasVisitedAll) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    EXPECT_FALSE(ant.hasVisitedAll());

    ant.visitCity(1, graph);
    EXPECT_FALSE(ant.hasVisitedAll());

    ant.visitCity(2, graph);
    EXPECT_TRUE(ant.hasVisitedAll());
}

// Test completeTour
TEST(AntTest, CompleteTour) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);
    ant.visitCity(2, graph);

    Tour tour = ant.completeTour(graph);

    EXPECT_EQ(tour.getSequence().size(), 3);
    EXPECT_EQ(tour.getSequence()[0], 0);
    EXPECT_EQ(tour.getSequence()[1], 1);
    EXPECT_EQ(tour.getSequence()[2], 2);

    // Distance: (0,0)->(3,0)=3.0 + (3,0)->(0,4)=5.0 + (0,4)->(0,0)=4.0 = 12.0
    EXPECT_DOUBLE_EQ(tour.getDistance(), 12.0);
}

// Test completeTour on incomplete tour throws exception
TEST(AntTest, CompleteTourIncomplete) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);

    EXPECT_THROW(ant.completeTour(graph), std::runtime_error);
}

// Test calculateTourLength
TEST(AntTest, CalculateTourLength) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    EXPECT_DOUBLE_EQ(ant.calculateTourLength(graph), 0.0);

    ant.visitCity(1, graph);
    EXPECT_DOUBLE_EQ(ant.calculateTourLength(graph), 3.0);

    ant.visitCity(2, graph);
    // Complete tour: includes return to start
    EXPECT_DOUBLE_EQ(ant.calculateTourLength(graph), 12.0);
}

// Test selectNextCity visits all cities
TEST(AntTest, SelectNextCityVisitsAll) {
    Graph graph = createSimpleGraph();
    PheromoneMatrix pheromones(3, 1.0);
    Ant ant(0, 3);

    std::set<int> visited;
    visited.insert(0);

    // Select next city twice to visit all cities
    for (int i = 0; i < 2; ++i) {
        int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 2.0);
        EXPECT_GE(nextCity, 0);
        EXPECT_LT(nextCity, 3);
        EXPECT_EQ(visited.count(nextCity), 0); // Not yet visited

        visited.insert(nextCity);
        ant.visitCity(nextCity, graph);
    }

    EXPECT_EQ(visited.size(), 3);
    EXPECT_TRUE(ant.hasVisitedAll());
}

// Test selectNextCity returns -1 when all cities visited
TEST(AntTest, SelectNextCityAllVisited) {
    Graph graph = createSimpleGraph();
    PheromoneMatrix pheromones(3, 1.0);
    Ant ant(0, 3);

    ant.visitCity(1, graph);
    ant.visitCity(2, graph);

    int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 2.0);
    EXPECT_EQ(nextCity, -1);
}

// Test selectNextCity with high pheromone on one edge
TEST(AntTest, SelectNextCityHighPheromone) {
    Graph graph = createSimpleGraph();
    PheromoneMatrix pheromones(3, 1.0);

    // Set very high pheromone on edge 0->1
    pheromones.setPheromone(0, 1, 100.0);

    // Run multiple times and count selections
    int city1Count = 0;
    int city2Count = 0;
    int trials = 100;

    for (int i = 0; i < trials; ++i) {
        Ant ant(0, 3);
        int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 2.0);

        if (nextCity == 1) city1Count++;
        else if (nextCity == 2) city2Count++;
    }

    // City 1 should be selected much more often due to high pheromone
    EXPECT_GT(city1Count, city2Count);
    EXPECT_GT(city1Count, trials / 2); // At least majority
}

// Test selectNextCity with zero distance handling
TEST(AntTest, SelectNextCityZeroDistance) {
    // Create graph with two cities at same location
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 0.0, 0.0),  // Same location as city 0
        City(2, 5.0, 0.0)
    };
    Graph graph(cities);
    PheromoneMatrix pheromones(3, 1.0);
    Ant ant(0, 3);

    // Should handle zero distance without crashing
    int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 2.0);
    EXPECT_GE(nextCity, 1);
    EXPECT_LE(nextCity, 2);
}

// Test selectNextCity with alpha = 0 (ignore pheromone)
TEST(AntTest, SelectNextCityAlphaZero) {
    Graph graph = createSimpleGraph();
    PheromoneMatrix pheromones(3, 1.0);

    // Set very high pheromone on edge 0->2
    pheromones.setPheromone(0, 2, 100.0);
    pheromones.setPheromone(0, 1, 1.0);

    // With alpha=0, pheromone should be ignored
    // City 1 is closer (distance 3.0 vs 4.0), so should be preferred
    int city1Count = 0;
    int city2Count = 0;
    int trials = 100;

    for (int i = 0; i < trials; ++i) {
        Ant ant(0, 3);
        int nextCity = ant.selectNextCity(graph, pheromones, 0.0, 2.0);

        if (nextCity == 1) city1Count++;
        else if (nextCity == 2) city2Count++;
    }

    // City 1 (closer) should be selected more often
    EXPECT_GT(city1Count, city2Count);
}

// Test selectNextCity with beta = 0 (ignore distance)
TEST(AntTest, SelectNextCityBetaZero) {
    Graph graph = createSimpleGraph();
    PheromoneMatrix pheromones(3, 1.0);

    // Set higher pheromone on edge to farther city
    pheromones.setPheromone(0, 2, 10.0);  // Farther city
    pheromones.setPheromone(0, 1, 1.0);   // Closer city

    // With beta=0, distance should be ignored
    int city1Count = 0;
    int city2Count = 0;
    int trials = 100;

    for (int i = 0; i < trials; ++i) {
        Ant ant(0, 3);
        int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 0.0);

        if (nextCity == 1) city1Count++;
        else if (nextCity == 2) city2Count++;
    }

    // City 2 (higher pheromone) should be selected more often
    EXPECT_GT(city2Count, city1Count);
}

// Test single city graph
TEST(AntTest, SingleCityGraph) {
    std::vector<City> cities = { City(0, 0.0, 0.0) };
    Graph graph(cities);
    Ant ant(0, 1);

    EXPECT_TRUE(ant.hasVisitedAll());

    Tour tour = ant.completeTour(graph);
    EXPECT_DOUBLE_EQ(tour.getDistance(), 0.0);
    EXPECT_EQ(tour.getSequence().size(), 1);
}

// Test two city graph
TEST(AntTest, TwoCityGraph) {
    std::vector<City> cities = {
        City(0, 0.0, 0.0),
        City(1, 5.0, 0.0)
    };
    Graph graph(cities);
    PheromoneMatrix pheromones(2, 1.0);
    Ant ant(0, 2);

    int nextCity = ant.selectNextCity(graph, pheromones, 1.0, 2.0);
    EXPECT_EQ(nextCity, 1);

    ant.visitCity(1, graph);

    Tour tour = ant.completeTour(graph);
    // Distance: 0->1 = 5.0, 1->0 = 5.0
    EXPECT_DOUBLE_EQ(tour.getDistance(), 10.0);
}

// Test tour sequence preservation
TEST(AntTest, TourSequencePreserved) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(2, graph);
    ant.visitCity(1, graph);

    const std::vector<int>& tour = ant.getTour();
    EXPECT_EQ(tour[0], 0);
    EXPECT_EQ(tour[1], 2);
    EXPECT_EQ(tour[2], 1);
}

// Test reset to different starting city
TEST(AntTest, ResetToDifferentStart) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);
    ant.reset(1);

    EXPECT_EQ(ant.getCurrentCity(), 1);
    EXPECT_EQ(ant.getTour().size(), 1);
    EXPECT_EQ(ant.getTour()[0], 1);
    EXPECT_TRUE(ant.hasVisited(1));
    EXPECT_FALSE(ant.hasVisited(0));
}

// Test calculateTourLength on incomplete tour
TEST(AntTest, CalculateTourLengthIncomplete) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);

    // Incomplete tour should not include return to start
    double calculatedLength = ant.calculateTourLength(graph);
    EXPECT_DOUBLE_EQ(calculatedLength, 3.0);
}

// Test multiple resets
TEST(AntTest, MultipleResets) {
    Graph graph = createSimpleGraph();
    Ant ant(0, 3);

    ant.visitCity(1, graph);
    ant.reset(0);
    ant.visitCity(2, graph);
    ant.reset(1);

    EXPECT_EQ(ant.getCurrentCity(), 1);
    EXPECT_EQ(ant.getTour().size(), 1);
    EXPECT_FALSE(ant.hasVisited(0));
    EXPECT_TRUE(ant.hasVisited(1));
    EXPECT_FALSE(ant.hasVisited(2));
}
