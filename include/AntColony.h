#ifndef ANTCOLONY_H
#define ANTCOLONY_H

#include <vector>
#include <functional>
#include "Graph.h"
#include "PheromoneMatrix.h"
#include "Ant.h"
#include "Tour.h"

class AntColony {
public:
    // Progress callback: iteration number, best distance so far
    using ProgressCallback = std::function<void(int, double)>;

    // Constructor
    AntColony(const Graph& graph, int numAnts, double alpha, double beta,
              double rho, double Q, bool useDistinctStartCities = false);

    // Initialize pheromones and ants
    void initialize();

    // Execute one complete iteration
    void runIteration();

    // All ants build tours
    void constructSolutions();

    // Evaporate and deposit pheromones
    void updatePheromones();

    // Run algorithm for specified iterations (or until convergence if maxIterations < 0)
    // If maxIterations < 0, runs until no improvement for 200 iterations
    Tour solve(int maxIterations, ProgressCallback callback = nullptr);

    // Get best solution found
    const Tour& getBestTour() const { return bestTour_; }

    // Get iteration history
    const std::vector<double>& getConvergenceData() const { return iterationBestDistances_; }

    // Getters for parameters
    int getNumAnts() const { return numAnts_; }
    double getAlpha() const { return alpha_; }
    double getBeta() const { return beta_; }
    double getRho() const { return rho_; }
    double getQ() const { return Q_; }

private:
    Graph graph_;
    PheromoneMatrix pheromones_;
    std::vector<Ant> ants_;
    int numAnts_;
    double alpha_;       // Pheromone importance
    double beta_;        // Heuristic importance
    double rho_;         // Evaporation rate
    double Q_;           // Pheromone deposit factor
    bool useDistinctStartCities_;  // If true, each ant starts at different city
    Tour bestTour_;
    std::vector<double> iterationBestDistances_;
};

#endif // ANTCOLONY_H
