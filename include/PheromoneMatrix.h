#ifndef PHEROMONEMATRIX_H
#define PHEROMONEMATRIX_H

#include <vector>

class PheromoneMatrix {
public:
    // Constructor
    PheromoneMatrix(int numCities, double initial);

    // Reset all pheromones to initial value
    void initialize(double value);

    // Get pheromone level between two cities
    double getPheromone(int cityA, int cityB) const;

    // Set pheromone level between two cities
    void setPheromone(int cityA, int cityB, double value);

    // Apply evaporation to all edges: pheromone *= (1 - rho)
    void evaporate(double rho);

    // Add pheromone to edge
    void depositPheromone(int cityA, int cityB, double amount);

    // Enforce min/max bounds (optional, for MMAS variant)
    void clampPheromones();

    // Getters
    int getNumCities() const { return numCities_; }
    double getMinPheromone() const { return minPheromone_; }
    double getMaxPheromone() const { return maxPheromone_; }

    // Setters for bounds
    void setMinPheromone(double minPheromone) { minPheromone_ = minPheromone; }
    void setMaxPheromone(double maxPheromone) { maxPheromone_ = maxPheromone; }

private:
    std::vector<std::vector<double>> pheromones_;
    int numCities_;
    double initialPheromone_;
    double minPheromone_;
    double maxPheromone_;
};

#endif // PHEROMONEMATRIX_H
