#include "PheromoneMatrix.h"
#include <algorithm>
#include <limits>

PheromoneMatrix::PheromoneMatrix(int numCities, double initial)
    : numCities_(numCities),
      initialPheromone_(initial),
      minPheromone_(0.0),
      maxPheromone_(std::numeric_limits<double>::max()) {
    // Initialize matrix with initial pheromone value
    pheromones_.resize(numCities_, std::vector<double>(numCities_, initial));
}

void PheromoneMatrix::initialize(double value) {
    for (int i = 0; i < numCities_; ++i) {
        for (int j = 0; j < numCities_; ++j) {
            pheromones_[i][j] = value;
        }
    }
}

double PheromoneMatrix::getPheromone(int cityA, int cityB) const {
    return pheromones_[cityA][cityB];
}

void PheromoneMatrix::setPheromone(int cityA, int cityB, double value) {
    pheromones_[cityA][cityB] = value;
    // Keep matrix symmetric for undirected TSP
    pheromones_[cityB][cityA] = value;
}

void PheromoneMatrix::evaporate(double rho) {
    for (int i = 0; i < numCities_; ++i) {
        for (int j = 0; j < numCities_; ++j) {
            pheromones_[i][j] *= (1.0 - rho);
        }
    }
}

void PheromoneMatrix::depositPheromone(int cityA, int cityB, double amount) {
    // Use atomic operations for thread safety when multiple ants may update same edge
    #ifdef _OPENMP
    #pragma omp atomic
    #endif
    pheromones_[cityA][cityB] += amount;

    // Keep matrix symmetric for undirected TSP (only if different cities)
    if (cityA != cityB) {
        #ifdef _OPENMP
        #pragma omp atomic
        #endif
        pheromones_[cityB][cityA] += amount;
    }
}

void PheromoneMatrix::clampPheromones() {
    for (int i = 0; i < numCities_; ++i) {
        for (int j = 0; j < numCities_; ++j) {
            pheromones_[i][j] = std::max(minPheromone_,
                                        std::min(maxPheromone_, pheromones_[i][j]));
        }
    }
}
