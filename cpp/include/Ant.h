#ifndef ANT_H
#define ANT_H

#include <vector>
#include <random>
#include "Graph.h"
#include "PheromoneMatrix.h"
#include "Tour.h"

class Ant {
public:
    // Constants
    static constexpr double EPSILON_DISTANCE = 1e-10;  // Minimum distance to avoid division by zero

    // Constructor
    Ant(int startCity, int numCities);

    // Reset ant to start a new tour
    void reset(int startCity);

    // Choose next city probabilistically based on pheromone and heuristic
    int selectNextCity(const Graph& graph, const PheromoneMatrix& pheromones,
                      double alpha, double beta);

    // Add city to tour
    void visitCity(int city, const Graph& graph);

    // Check if tour is complete
    bool hasVisitedAll() const;

    // Finalize and return the tour
    Tour completeTour(const Graph& graph);

    // Calculate total tour distance
    double calculateTourLength(const Graph& graph) const;

    // Getters
    int getCurrentCity() const { return currentCity_; }
    const std::vector<int>& getTour() const { return tour_; }
    double getTourLength() const { return tourLength_; }
    bool hasVisited(int city) const { return visited_[city]; }

private:
    int currentCity_;
    std::vector<bool> visited_;
    std::vector<int> tour_;
    double tourLength_;
    int numCities_;

    // Shared random number generator for all ants
    static std::mt19937& getRandomGenerator();
};

#endif // ANT_H
