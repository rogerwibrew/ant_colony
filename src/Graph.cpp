/**
 * @file Graph.cpp
 * @brief Implementation of the Graph class
 */

#include "Graph.h"

/**
 * Main constructor - builds the graph and precomputes all distances.
 * Uses member initializer list for efficiency.
 */
Graph::Graph(const std::vector<City>& cities)
    : cities_(cities), numCities_(cities.size()) {
    buildDistanceMatrix();
}

/**
 * Default constructor for empty graph.
 * Useful when file loading fails or for initialization before loading.
 */
Graph::Graph() : numCities_(0) {}

/**
 * Build the distance matrix by computing Euclidean distances between
 * all pairs of cities. This is done once at construction to enable
 * O(1) distance lookups during the ACO algorithm.
 *
 * Optimization: Only compute upper triangle and mirror to lower triangle
 * since the distance matrix is symmetric (distance[i][j] == distance[j][i])
 * for undirected TSP.
 *
 * Time complexity: O(n²) where n is the number of cities
 * Space complexity: O(n²) for the distance matrix
 */
void Graph::buildDistanceMatrix() {
    // Allocate n×n matrix initialized with zeros
    distanceMatrix_.resize(numCities_, std::vector<double>(numCities_, 0.0));

    // Calculate distances between all pairs of cities
    // Note: diagonal (i==i) remains 0.0 (distance from city to itself)
    for (int i = 0; i < numCities_; ++i) {
        for (int j = i + 1; j < numCities_; ++j) {
            // Compute distance once using City::distanceTo()
            double distance = cities_[i].distanceTo(cities_[j]);
            // Store in both positions for symmetric matrix
            distanceMatrix_[i][j] = distance;
            distanceMatrix_[j][i] = distance;
        }
    }
}

/**
 * Get the precomputed distance between two cities.
 * Includes bounds checking to prevent invalid memory access.
 *
 * @return Distance between cities, or 0.0 if indices are out of bounds
 */
double Graph::getDistance(int cityA, int cityB) const {
    // Validate indices to prevent out-of-bounds access
    if (cityA < 0 || cityA >= numCities_ || cityB < 0 || cityB >= numCities_) {
        return 0.0; // Basic error handling - return 0 for invalid indices
    }
    return distanceMatrix_[cityA][cityB];
}

// Return total number of cities in the problem
int Graph::getNumCities() const {
    return numCities_;
}

// Return reference to city at given index (no bounds checking)
const City& Graph::getCity(int index) const {
    return cities_[index];
}

// Return reference to entire cities vector
const std::vector<City>& Graph::getCities() const {
    return cities_;
}

// Check if graph is valid (contains at least one city)
bool Graph::isValid() const {
    return numCities_ > 0;
}
