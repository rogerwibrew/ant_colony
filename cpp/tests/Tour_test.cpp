/**
 * @file Tour_test.cpp
 * @brief Unit tests for the Tour class
 */

#include "Tour.h"
#include <gtest/gtest.h>

// Test default constructor creates invalid tour
TEST(TourTest, DefaultConstructor) {
    Tour tour;
    EXPECT_EQ(tour.getDistance(), 0.0);
    EXPECT_TRUE(tour.getSequence().empty());
    EXPECT_FALSE(tour.validate(5));  // Empty tour is invalid
}

// Test parameterized constructor
TEST(TourTest, ParameterizedConstructor) {
    std::vector<int> sequence = {0, 1, 2, 3, 4};
    double distance = 123.45;

    Tour tour(sequence, distance);

    EXPECT_EQ(tour.getDistance(), distance);
    EXPECT_EQ(tour.getSequence(), sequence);
    EXPECT_TRUE(tour.validate(5));
}

// Test setTour method
TEST(TourTest, SetTour) {
    Tour tour;
    std::vector<int> sequence = {0, 2, 1};
    double distance = 50.0;

    tour.setTour(sequence, distance);

    EXPECT_EQ(tour.getDistance(), distance);
    EXPECT_EQ(tour.getSequence(), sequence);
    EXPECT_TRUE(tour.validate(3));
}

// Test validation with correct tour
TEST(TourTest, ValidateTourCorrect) {
    std::vector<int> sequence = {0, 3, 1, 4, 2};
    Tour tour(sequence, 100.0);

    EXPECT_TRUE(tour.validate(5));
}

// Test validation fails with wrong number of cities
TEST(TourTest, ValidateTourWrongSize) {
    std::vector<int> sequence = {0, 1, 2};
    Tour tour(sequence, 50.0);

    EXPECT_TRUE(tour.validate(3));   // Correct size
    EXPECT_FALSE(tour.validate(5));  // Wrong size
    EXPECT_FALSE(tour.validate(2));  // Wrong size
}

// Test validation fails with duplicate cities
TEST(TourTest, ValidateTourDuplicateCities) {
    std::vector<int> sequence = {0, 1, 2, 1, 3};  // City 1 appears twice
    Tour tour(sequence, 100.0);

    EXPECT_FALSE(tour.validate(5));
}

// Test validation fails with city ID out of range (negative)
TEST(TourTest, ValidateTourNegativeCityId) {
    std::vector<int> sequence = {0, 1, -1, 3, 4};
    Tour tour(sequence, 100.0);

    EXPECT_FALSE(tour.validate(5));
}

// Test validation fails with city ID out of range (too high)
TEST(TourTest, ValidateTourCityIdTooHigh) {
    std::vector<int> sequence = {0, 1, 2, 3, 5};  // City 5 is out of range [0-4]
    Tour tour(sequence, 100.0);

    EXPECT_FALSE(tour.validate(5));
}

// Test validation with single city
TEST(TourTest, ValidateSingleCity) {
    std::vector<int> sequence = {0};
    Tour tour(sequence, 0.0);

    EXPECT_TRUE(tour.validate(1));
}

// Test validation with two cities
TEST(TourTest, ValidateTwoCities) {
    std::vector<int> sequence = {0, 1};
    Tour tour(sequence, 10.0);

    EXPECT_TRUE(tour.validate(2));
}

// Test that sequence order is preserved
TEST(TourTest, SequenceOrderPreserved) {
    std::vector<int> sequence = {4, 2, 0, 3, 1};
    Tour tour(sequence, 75.0);

    const std::vector<int>& retrievedSequence = tour.getSequence();
    EXPECT_EQ(retrievedSequence.size(), 5);
    EXPECT_EQ(retrievedSequence[0], 4);
    EXPECT_EQ(retrievedSequence[1], 2);
    EXPECT_EQ(retrievedSequence[2], 0);
    EXPECT_EQ(retrievedSequence[3], 3);
    EXPECT_EQ(retrievedSequence[4], 1);
}

// Test updating an existing tour
TEST(TourTest, UpdateExistingTour) {
    Tour tour({0, 1, 2}, 50.0);

    EXPECT_EQ(tour.getDistance(), 50.0);
    EXPECT_EQ(tour.getSequence().size(), 3);

    tour.setTour({0, 1, 2, 3, 4}, 120.0);

    EXPECT_EQ(tour.getDistance(), 120.0);
    EXPECT_EQ(tour.getSequence().size(), 5);
    EXPECT_TRUE(tour.validate(5));
}

// Test validation with all cities missing
TEST(TourTest, ValidateAllCitiesMissing) {
    std::vector<int> sequence = {0, 0, 0, 0, 0};  // Only city 0, repeated
    Tour tour(sequence, 0.0);

    EXPECT_FALSE(tour.validate(5));
}

// Test distance can be zero (valid for degenerate case)
TEST(TourTest, ZeroDistance) {
    std::vector<int> sequence = {0, 1, 2};
    Tour tour(sequence, 0.0);

    EXPECT_EQ(tour.getDistance(), 0.0);
    EXPECT_TRUE(tour.validate(3));
}

// Test distance can be negative (though unusual, not prevented)
TEST(TourTest, NegativeDistance) {
    std::vector<int> sequence = {0, 1, 2};
    Tour tour(sequence, -10.0);

    EXPECT_EQ(tour.getDistance(), -10.0);
    EXPECT_TRUE(tour.validate(3));
}
