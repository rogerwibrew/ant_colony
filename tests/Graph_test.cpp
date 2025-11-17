#include <gtest/gtest.h>
#include "Graph.h"
#include "City.h"
#include <vector>

// Test empty graph
TEST(GraphTest, EmptyGraph) {
    Graph graph;

    EXPECT_EQ(graph.getNumCities(), 0);
    EXPECT_FALSE(graph.isValid());
}

// Test graph with single city
TEST(GraphTest, SingleCity) {
    std::vector<City> cities;
    cities.push_back(City(0, 10.0, 20.0));

    Graph graph(cities);

    EXPECT_EQ(graph.getNumCities(), 1);
    EXPECT_TRUE(graph.isValid());
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 0), 0.0);
}

// Test graph with two cities
TEST(GraphTest, TwoCities) {
    std::vector<City> cities;
    cities.push_back(City(0, 0.0, 0.0));
    cities.push_back(City(1, 3.0, 4.0));

    Graph graph(cities);

    EXPECT_EQ(graph.getNumCities(), 2);
    EXPECT_TRUE(graph.isValid());

    // Check distance
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 1), 5.0);
    EXPECT_DOUBLE_EQ(graph.getDistance(1, 0), 5.0); // Symmetry
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(graph.getDistance(1, 1), 0.0);
}

// Test graph with multiple cities - distance matrix symmetry
TEST(GraphTest, DistanceMatrixSymmetry) {
    std::vector<City> cities;
    cities.push_back(City(0, 0.0, 0.0));
    cities.push_back(City(1, 1.0, 0.0));
    cities.push_back(City(2, 1.0, 1.0));
    cities.push_back(City(3, 0.0, 1.0));

    Graph graph(cities);

    EXPECT_EQ(graph.getNumCities(), 4);

    // Check all symmetries
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_DOUBLE_EQ(graph.getDistance(i, j), graph.getDistance(j, i));
        }
    }
}

// Test graph distance calculations
TEST(GraphTest, CorrectDistanceCalculations) {
    std::vector<City> cities;
    cities.push_back(City(0, 0.0, 0.0));
    cities.push_back(City(1, 3.0, 0.0));
    cities.push_back(City(2, 3.0, 4.0));

    Graph graph(cities);

    EXPECT_DOUBLE_EQ(graph.getDistance(0, 1), 3.0);  // Horizontal
    EXPECT_DOUBLE_EQ(graph.getDistance(1, 2), 4.0);  // Vertical
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 2), 5.0);  // Diagonal (3-4-5 triangle)
}

// Test getCity method
TEST(GraphTest, GetCity) {
    std::vector<City> cities;
    cities.push_back(City(0, 10.0, 20.0));
    cities.push_back(City(1, 30.0, 40.0));

    Graph graph(cities);

    const City& city0 = graph.getCity(0);
    EXPECT_EQ(city0.getId(), 0);
    EXPECT_DOUBLE_EQ(city0.getX(), 10.0);
    EXPECT_DOUBLE_EQ(city0.getY(), 20.0);

    const City& city1 = graph.getCity(1);
    EXPECT_EQ(city1.getId(), 1);
    EXPECT_DOUBLE_EQ(city1.getX(), 30.0);
    EXPECT_DOUBLE_EQ(city1.getY(), 40.0);
}

// Test getCities method
TEST(GraphTest, GetCities) {
    std::vector<City> cities;
    cities.push_back(City(0, 10.0, 20.0));
    cities.push_back(City(1, 30.0, 40.0));
    cities.push_back(City(2, 50.0, 60.0));

    Graph graph(cities);

    const std::vector<City>& retrievedCities = graph.getCities();
    EXPECT_EQ(retrievedCities.size(), 3);

    for (size_t i = 0; i < cities.size(); ++i) {
        EXPECT_EQ(retrievedCities[i].getId(), cities[i].getId());
        EXPECT_DOUBLE_EQ(retrievedCities[i].getX(), cities[i].getX());
        EXPECT_DOUBLE_EQ(retrievedCities[i].getY(), cities[i].getY());
    }
}

// Test invalid indices (basic error handling)
TEST(GraphTest, InvalidIndices) {
    std::vector<City> cities;
    cities.push_back(City(0, 0.0, 0.0));
    cities.push_back(City(1, 1.0, 1.0));

    Graph graph(cities);

    // Out of bounds - should return 0.0
    EXPECT_DOUBLE_EQ(graph.getDistance(-1, 0), 0.0);
    EXPECT_DOUBLE_EQ(graph.getDistance(0, -1), 0.0);
    EXPECT_DOUBLE_EQ(graph.getDistance(2, 0), 0.0);
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 2), 0.0);
}
