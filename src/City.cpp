/**
 * @file City.cpp
 * @brief Implementation of the City class
 */

#include "City.h"

// Constructor using member initializer list for efficiency
City::City(int id, double x, double y) : id_(id), x_(x), y_(y) {}

/**
 * Calculate the straight-line (Euclidean) distance between two cities.
 * This is the most common distance metric for TSP problems.
 *
 * Time complexity: O(1)
 */
double City::distanceTo(const City& other) const {
    double dx = x_ - other.x_;  // Calculate horizontal difference
    double dy = y_ - other.y_;  // Calculate vertical difference
    // Apply Pythagorean theorem: distance = sqrt(dx^2 + dy^2)
    return std::sqrt(dx * dx + dy * dy);
}

// Simple getter - returns city ID
int City::getId() const {
    return id_;
}

// Simple getter - returns X coordinate
double City::getX() const {
    return x_;
}

// Simple getter - returns Y coordinate
double City::getY() const {
    return y_;
}
