#include "AntColony.h"
#include <limits>
#include <random>
#include <algorithm>

AntColony::AntColony(const Graph& graph, int numAnts, double alpha, double beta,
                     double rho, double Q, bool useDistinctStartCities)
    : graph_(graph),
      pheromones_(graph.getNumCities(), 1.0),  // Will be properly initialized in initialize()
      numAnts_(numAnts),
      alpha_(alpha),
      beta_(beta),
      rho_(rho),
      Q_(Q),
      useDistinctStartCities_(useDistinctStartCities),
      bestTour_(std::vector<int>(), std::numeric_limits<double>::max()) {
    // Create ants
    ants_.reserve(numAnts);
}

void AntColony::initialize() {
    // Calculate initial pheromone value using τ₀ = m / C^nn
    // where m is the number of ants and C^nn is the nearest neighbor tour length
    double nearestNeighborLength = graph_.nearestNeighborTourLength();
    double initialPheromone = 1.0;  // Default fallback

    if (nearestNeighborLength > 0.0) {
        initialPheromone = static_cast<double>(numAnts_) / nearestNeighborLength;
    }

    // Initialize pheromone matrix with calculated value
    pheromones_.initialize(initialPheromone);

    // Clear iteration history
    iterationBestDistances_.clear();

    // Reset best tour
    bestTour_ = Tour(std::vector<int>(), std::numeric_limits<double>::max());
}

void AntColony::constructSolutions() {
    int numCities = graph_.getNumCities();

    // Clear ants and recreate them
    ants_.clear();

    if (useDistinctStartCities_) {
        // Assign each ant to a different starting city
        // If numAnts <= numCities, each ant gets a unique city
        // If numAnts > numCities, use cyclic assignment
        for (int i = 0; i < numAnts_; ++i) {
            int startCity = i % numCities;
            ants_.emplace_back(startCity, numCities);
        }
    } else {
        // Create ants starting from random cities
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, numCities - 1);

        for (int i = 0; i < numAnts_; ++i) {
            int startCity = dist(gen);
            ants_.emplace_back(startCity, numCities);
        }
    }

    // Each ant constructs a complete tour
    for (auto& ant : ants_) {
        while (!ant.hasVisitedAll()) {
            int nextCity = ant.selectNextCity(graph_, pheromones_, alpha_, beta_);

            if (nextCity == -1) {
                // No more cities to visit (should not happen in normal operation)
                break;
            }

            ant.visitCity(nextCity, graph_);
        }
    }
}

void AntColony::updatePheromones() {
    // Evaporate pheromones
    pheromones_.evaporate(rho_);

    // Deposit pheromones from all ants
    for (auto& ant : ants_) {
        if (!ant.hasVisitedAll()) {
            continue; // Skip incomplete tours
        }

        Tour tour = ant.completeTour(graph_);
        double tourLength = tour.getDistance();

        // Pheromone deposit amount: Q / tourLength
        double depositAmount = Q_ / tourLength;

        const std::vector<int>& tourSequence = tour.getSequence();

        // Deposit pheromones on each edge in the tour
        for (size_t i = 0; i < tourSequence.size(); ++i) {
            int cityA = tourSequence[i];
            int cityB = tourSequence[(i + 1) % tourSequence.size()]; // Wrap around to start

            pheromones_.depositPheromone(cityA, cityB, depositAmount);
        }
    }
}

void AntColony::runIteration() {
    // Construct solutions
    constructSolutions();

    // Find best tour in this iteration
    double iterationBest = std::numeric_limits<double>::max();

    for (auto& ant : ants_) {
        if (!ant.hasVisitedAll()) {
            continue;
        }

        Tour tour = ant.completeTour(graph_);
        double tourLength = tour.getDistance();

        if (tourLength < iterationBest) {
            iterationBest = tourLength;
        }

        // Update global best
        if (tourLength < bestTour_.getDistance()) {
            bestTour_ = tour;
        }
    }

    // Record iteration best
    iterationBestDistances_.push_back(iterationBest);

    // Update pheromones
    updatePheromones();
}

Tour AntColony::solve(int maxIterations, ProgressCallback callback) {
    initialize();

    if (maxIterations < 0) {
        // Run until no improvement for 200 iterations
        const int NO_IMPROVEMENT_LIMIT = 200;
        int iterationsWithoutImprovement = 0;
        int iteration = 0;
        double lastBestDistance = std::numeric_limits<double>::max();

        while (iterationsWithoutImprovement < NO_IMPROVEMENT_LIMIT) {
            runIteration();
            iteration++;

            double currentBestDistance = bestTour_.getDistance();

            // Check if we found a better solution
            if (currentBestDistance < lastBestDistance) {
                iterationsWithoutImprovement = 0;
                lastBestDistance = currentBestDistance;
            } else {
                iterationsWithoutImprovement++;
            }

            // Call progress callback if provided
            if (callback) {
                callback(iteration, currentBestDistance);
            }
        }
    } else {
        // Run for fixed number of iterations
        for (int i = 0; i < maxIterations; ++i) {
            runIteration();

            // Call progress callback if provided
            if (callback) {
                callback(i + 1, bestTour_.getDistance());
            }
        }
    }

    return bestTour_;
}
