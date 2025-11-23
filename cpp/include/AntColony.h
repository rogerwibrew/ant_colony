#ifndef ANTCOLONY_H
#define ANTCOLONY_H

#include <vector>
#include <random>
#include <functional>
#include "Graph.h"
#include "PheromoneMatrix.h"
#include "Ant.h"
#include "Tour.h"
#include "LocalSearch.h"

class AntColony {
public:
    // Progress callback: iteration number, best distance, best tour sequence, convergence history
    using ProgressCallback = std::function<void(int, double, const std::vector<int>&, const std::vector<double>&)>;

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
    // If maxIterations < 0, runs until no improvement for convergenceThreshold_ iterations
    Tour solve(int maxIterations, ProgressCallback callback = nullptr);

    // Set progress callback (alternative to passing to solve())
    void setProgressCallback(ProgressCallback callback);

    // Set callback interval (default: 10 iterations)
    void setCallbackInterval(int interval);

    // Set convergence threshold (default: 200 iterations without improvement)
    void setConvergenceThreshold(int threshold);

    // Enable/disable parallel execution (default: true if OpenMP available)
    void setUseParallel(bool useParallel);

    // Set number of threads for parallel execution (0 = auto-detect, 1 = serial, 2+ = specific count)
    // Only effective if OpenMP is available and useParallel is true
    void setNumThreads(int numThreads);

    // Enable/disable local search optimization (default: disabled)
    void setUseLocalSearch(bool useLocalSearch);

    // Enable/disable 3-opt in addition to 2-opt (default: true)
    // Only effective if local search is enabled
    void setUse3Opt(bool use3opt);

    // Set when to apply local search: "best" (only to best tour), "all" (to all ant tours), or "none" (disabled)
    // Default: "best"
    void setLocalSearchMode(const std::string& mode);

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
    ProgressCallback progressCallback_;  // Callback for progress updates
    int callbackInterval_ = 10;          // Invoke callback every N iterations
    int convergenceThreshold_ = 200;     // Iterations without improvement before stopping

    // Threading control
    bool useParallel_ = true;            // Enable parallel execution (if OpenMP available)
    int numThreads_ = 0;                 // Number of threads (0 = auto, 1 = serial, 2+ = specific)

    // Local search control
    bool useLocalSearch_ = false;        // Enable local search (2-opt/3-opt)
    bool use3opt_ = true;                // Use 3-opt in addition to 2-opt
    std::string localSearchMode_ = "best";  // "best", "all", or "none"

    // Store constructed/improved tours for pheromone updates
    std::vector<Tour> antTours_;         // Tours from each ant (possibly improved by local search)

    // Shared random number generator for colony
    static std::mt19937& getRandomGenerator();
};

#endif // ANTCOLONY_H
