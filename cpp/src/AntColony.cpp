#include "AntColony.h"
#include <limits>
#include <random>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

// Shared random number generator for AntColony
std::mt19937& AntColony::getRandomGenerator() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return gen;
}

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
        std::uniform_int_distribution<> dist(0, numCities - 1);

        for (int i = 0; i < numAnts_; ++i) {
            int startCity = dist(getRandomGenerator());
            ants_.emplace_back(startCity, numCities);
        }
    }

    // Each ant constructs a complete tour
    // Parallelize this loop - each ant operates independently
    #ifdef _OPENMP
    #pragma omp parallel for schedule(dynamic) if(useParallel_ && numAnts_ >= 8)
    #endif
    for (int i = 0; i < numAnts_; ++i) {
        Ant& ant = ants_[i];
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

    // Deposit pheromones from all ants - parallelize this loop
    // Use adaptive threading: cap threads at 2× ants to reduce atomic contention
    #ifdef _OPENMP
    int max_threads = omp_get_max_threads();
    int effective_threads = std::min(max_threads, static_cast<int>(ants_.size()) * 2);
    #pragma omp parallel for num_threads(effective_threads) if(useParallel_ && ants_.size() >= 8)
    #endif
    for (size_t antIdx = 0; antIdx < ants_.size(); ++antIdx) {
        if (!ants_[antIdx].hasVisitedAll()) {
            continue; // Skip incomplete tours
        }

        Tour tour = ants_[antIdx].completeTour(graph_);
        double tourLength = tour.getDistance();

        // Pheromone deposit amount: Q / tourLength
        double depositAmount = Q_ / tourLength;

        const std::vector<int>& tourSequence = tour.getSequence();

        // Deposit pheromones on each edge in the tour
        // depositPheromone already uses atomic operations for thread safety
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
    Tour iterationBestTour;

    // Process all ants to find the best - can be parallelized
    #ifdef _OPENMP
    if (useParallel_ && ants_.size() >= 8) {
        #pragma omp parallel
        {
            // Each thread tracks its own best
            double threadBest = std::numeric_limits<double>::max();
            Tour threadBestTour;

            #pragma omp for nowait
            for (size_t i = 0; i < ants_.size(); ++i) {
                if (!ants_[i].hasVisitedAll()) {
                    continue;
                }

                Tour tour = ants_[i].completeTour(graph_);
                double tourLength = tour.getDistance();

                if (tourLength < threadBest) {
                    threadBest = tourLength;
                    threadBestTour = tour;
                }
            }

            // Merge thread results into global best
            #pragma omp critical
            {
                if (threadBest < iterationBest) {
                    iterationBest = threadBest;
                    iterationBestTour = threadBestTour;
                }
                if (threadBest < bestTour_.getDistance()) {
                    bestTour_ = threadBestTour;
                }
            }
        }
    } else {
    #endif
        // Serial version (no OpenMP or disabled)
        for (auto& ant : ants_) {
            if (!ant.hasVisitedAll()) {
                continue;
            }

            Tour tour = ant.completeTour(graph_);
            double tourLength = tour.getDistance();

            if (tourLength < iterationBest) {
                iterationBest = tourLength;
                iterationBestTour = tour;
            }

            // Update global best
            if (tourLength < bestTour_.getDistance()) {
                bestTour_ = tour;
            }
        }
    #ifdef _OPENMP
    }
    #endif

    // Record iteration best
    iterationBestDistances_.push_back(iterationBest);

    // Update pheromones
    updatePheromones();
}

Tour AntColony::solve(int maxIterations, ProgressCallback callback) {
    initialize();

    if (maxIterations < 0) {
        // Run until no improvement for convergenceThreshold_ iterations
        int iterationsWithoutImprovement = 0;
        int iteration = 0;
        double lastBestDistance = std::numeric_limits<double>::max();

        while (iterationsWithoutImprovement < convergenceThreshold_) {
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

            // Call progress callback if provided and at the right interval
            ProgressCallback activeCallback = callback ? callback : progressCallback_;
            if (activeCallback && (iteration % callbackInterval_ == 0)) {
                activeCallback(iteration, currentBestDistance,
                             bestTour_.getSequence(), iterationBestDistances_);
            }
        }
    } else {
        // Run for fixed number of iterations
        for (int i = 0; i < maxIterations; ++i) {
            runIteration();

            // Call progress callback if provided and at the right interval
            int iteration = i + 1;
            ProgressCallback activeCallback = callback ? callback : progressCallback_;
            if (activeCallback && (iteration % callbackInterval_ == 0)) {
                activeCallback(iteration, bestTour_.getDistance(),
                             bestTour_.getSequence(), iterationBestDistances_);
            }
        }
    }

    return bestTour_;
}

void AntColony::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

void AntColony::setCallbackInterval(int interval) {
    callbackInterval_ = interval;
}

void AntColony::setConvergenceThreshold(int threshold) {
    convergenceThreshold_ = threshold;
}

void AntColony::setUseParallel(bool useParallel) {
    useParallel_ = useParallel;
}

void AntColony::setNumThreads(int numThreads) {
    numThreads_ = numThreads;
#ifdef _OPENMP
    if (numThreads > 0) {
        omp_set_num_threads(numThreads);
    }
#endif
}
