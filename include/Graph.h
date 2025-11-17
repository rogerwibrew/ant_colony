/**
 * @file Graph.h
 * @brief Represents the complete TSP problem instance
 *
 * This class stores all cities in the problem and maintains a precomputed
 * distance matrix for O(1) distance lookups between any two cities.
 */

#ifndef GRAPH_H
#define GRAPH_H

#include "City.h"
#include <vector>

/**
 * @class Graph
 * @brief Complete representation of the TSP problem with cities and distances
 *
 * The Graph precomputes and caches all pairwise distances between cities
 * for efficient access during the ACO algorithm. The distance matrix is
 * symmetric (distance[i][j] == distance[j][i]) for undirected TSP.
 */
class Graph {
public:
    /**
     * @brief Construct a graph from a list of cities
     * @param cities Vector of City objects representing the problem instance
     *
     * The constructor automatically builds the distance matrix by computing
     * Euclidean distances between all pairs of cities.
     * Marked explicit to prevent implicit conversions.
     */
    explicit Graph(const std::vector<City>& cities);

    /**
     * @brief Default constructor creating an empty graph
     *
     * Useful for error handling when file loading fails.
     */
    Graph();

    /**
     * @brief Get the distance between two cities by their indices
     * @param cityA Index of first city (0-based)
     * @param cityB Index of second city (0-based)
     * @return double The distance between the cities, or 0.0 if indices invalid
     *
     * Time complexity: O(1) - direct matrix lookup
     */
    double getDistance(int cityA, int cityB) const;

    /**
     * @brief Get the total number of cities in the graph
     * @return int Number of cities
     */
    int getNumCities() const;

    /**
     * @brief Get a city by its index
     * @param index The city index (0-based)
     * @return const City& Reference to the city at the given index
     *
     * Note: No bounds checking - caller must ensure valid index
     */
    const City& getCity(int index) const;

    /**
     * @brief Get all cities in the graph
     * @return const std::vector<City>& Reference to the cities vector
     */
    const std::vector<City>& getCities() const;

    /**
     * @brief Check if the graph contains cities
     * @return true if graph has at least one city, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Calculate tour length using nearest neighbor heuristic
     * @param startCity Starting city index (default: 0)
     * @return double The tour length using greedy nearest neighbor approach
     *
     * This is used to compute a reasonable initial pheromone value.
     * The nearest neighbor heuristic builds a tour by always visiting
     * the closest unvisited city next.
     */
    double nearestNeighborTourLength(int startCity = 0) const;

private:
    std::vector<City> cities_;                          ///< All cities in the problem
    std::vector<std::vector<double>> distanceMatrix_;   ///< Precomputed n×n distance matrix
    int numCities_;                                     ///< Number of cities (cached for efficiency)

    /**
     * @brief Build the symmetric distance matrix
     *
     * Computes distances between all pairs of cities. Since the matrix is
     * symmetric, only the upper triangle is computed and then mirrored.
     * Time complexity: O(n²) where n is the number of cities
     */
    void buildDistanceMatrix();
};

#endif // GRAPH_H
