#include <gtest/gtest.h>
#include "City.h"
#include <cmath>

// Test constructor and getters
TEST(CityTest, ConstructorAndGetters) {
    City city(0, 10.0, 20.0);

    EXPECT_EQ(city.getId(), 0);
    EXPECT_DOUBLE_EQ(city.getX(), 10.0);
    EXPECT_DOUBLE_EQ(city.getY(), 20.0);
}

// Test distance calculation with horizontal distance
TEST(CityTest, HorizontalDistance) {
    City city1(0, 0.0, 0.0);
    City city2(1, 3.0, 0.0);

    EXPECT_DOUBLE_EQ(city1.distanceTo(city2), 3.0);
    EXPECT_DOUBLE_EQ(city2.distanceTo(city1), 3.0); // Symmetry
}

// Test distance calculation with vertical distance
TEST(CityTest, VerticalDistance) {
    City city1(0, 0.0, 0.0);
    City city2(1, 0.0, 4.0);

    EXPECT_DOUBLE_EQ(city1.distanceTo(city2), 4.0);
    EXPECT_DOUBLE_EQ(city2.distanceTo(city1), 4.0); // Symmetry
}

// Test distance calculation with diagonal (3-4-5 triangle)
TEST(CityTest, DiagonalDistance) {
    City city1(0, 0.0, 0.0);
    City city2(1, 3.0, 4.0);

    EXPECT_DOUBLE_EQ(city1.distanceTo(city2), 5.0);
    EXPECT_DOUBLE_EQ(city2.distanceTo(city1), 5.0); // Symmetry
}

// Test distance to self is zero
TEST(CityTest, DistanceToSelf) {
    City city(0, 10.0, 20.0);

    EXPECT_DOUBLE_EQ(city.distanceTo(city), 0.0);
}

// Test with negative coordinates
TEST(CityTest, NegativeCoordinates) {
    City city1(0, -5.0, -5.0);
    City city2(1, -2.0, -1.0);

    double expected = std::sqrt(9.0 + 16.0); // sqrt((3)^2 + (4)^2) = 5.0
    EXPECT_DOUBLE_EQ(city1.distanceTo(city2), expected);
}

// Test with floating point coordinates
TEST(CityTest, FloatingPointCoordinates) {
    City city1(0, 1.5, 2.5);
    City city2(1, 4.5, 6.5);

    double expected = std::sqrt(9.0 + 16.0); // sqrt((3)^2 + (4)^2) = 5.0
    EXPECT_DOUBLE_EQ(city1.distanceTo(city2), expected);
}
