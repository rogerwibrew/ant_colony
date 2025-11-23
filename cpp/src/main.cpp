/**
 * @file main.cpp
 * @brief Entry point for the Ant Colony Optimization TSP solver
 *
 * This program will solve the Travelling Salesman Problem using
 * the Ant Colony Optimization metaheuristic algorithm.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include "TSPLoader.h"
#include "AntColony.h"
#include "Graph.h"
#include "Tour.h"

#ifdef _OPENMP
#include <omp.h>
#endif

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input_file> [options]\n";
    std::cout << "\nAlgorithm Options:\n";
    std::cout << "  --ants <n>       Number of ants (default: number of cities)\n";
    std::cout << "  --iterations <n> Number of iterations (default: until no improvement for 200 iterations)\n";
    std::cout << "  --alpha <f>      Pheromone importance (default: 1.0)\n";
    std::cout << "  --beta <f>       Heuristic importance (default: 2.0)\n";
    std::cout << "  --rho <f>        Evaporation rate (default: 0.5)\n";
    std::cout << "  --Q <f>          Pheromone deposit factor (default: 100.0)\n";
    std::cout << "\nElitist Strategy Options:\n";
    std::cout << "  --elitist        Enable elitist pheromone deposits (default: disabled)\n";
    std::cout << "  --elitist-weight <f> Weight for elitist deposits (default: numAnts)\n";
    std::cout << "  --pheromone-mode <mode> Pheromone update strategy:\n";
    std::cout << "                   'all' (all ants, default), 'best-iteration' (iteration best),\n";
    std::cout << "                   'best-so-far' (global best), 'rank' (top-k ants)\n";
    std::cout << "  --rank-size <n>  Number of elite ants for rank mode (default: numAnts/2)\n";
    std::cout << "\nLocal Search Options:\n";
    std::cout << "  --local-search   Enable 2-opt/3-opt local search (default: disabled)\n";
    std::cout << "  --2opt-only      Use only 2-opt (skip 3-opt, default: use both)\n";
    std::cout << "  --ls-mode <mode> When to apply: 'best' (only best tour), 'all' (all tours), 'none' (default: best)\n";
    std::cout << "\nThreading Options:\n";
    std::cout << "  --threads <n>    Number of threads (0=auto, 1=serial, 2+=specific, default: 0)\n";
    std::cout << "  --serial         Force single-threaded execution (same as --threads 1)\n";
    std::cout << "\nInput file format:\n";
    std::cout << "  Coordinate format: n\\n id x y\\n ...\n";
    std::cout << "  Distance matrix format: n\\n d00 d01 ...\\n d10 d11 ...\\n ...\n";
}

/**
 * @brief Main entry point
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @return int Exit status (0 for success)
 *
 * Expected usage:
 *   ./ant_colony_tsp [input_file]
 *
 * Where input_file is a TSP problem in coordinate or distance matrix format.
 */
int main(int argc, char** argv) {
    // Default parameters
    std::string inputFile;
    int numAnts = -1;  // -1 means use number of cities (set after loading graph)
    int iterations = -1;  // -1 means run until no improvement for 200 iterations
    double alpha = 1.0;
    double beta = 2.0;
    double rho = 0.5;
    double Q = 100.0;
    int numThreads = 0;  // 0 = auto-detect, 1 = serial, 2+ = specific count
    bool useParallel = true;  // Enable parallel execution by default (if OpenMP available)
    bool useLocalSearch = false;  // Enable local search (2-opt/3-opt)
    bool use3opt = true;  // Use 3-opt in addition to 2-opt
    std::string localSearchMode = "best";  // "best", "all", or "none"
    bool useElitist = false;  // Enable elitist strategy
    double elitistWeight = -1.0;  // -1 means use numAnts (set after loading graph)
    std::string pheromoneMode = "all";  // "all", "best-iteration", "best-so-far", "rank"
    int rankSize = -1;  // -1 means use numAnts/2 (auto)

    // Parse command-line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    inputFile = argv[1];

    // Parse optional arguments
    for (int i = 2; i < argc; ) {
        std::string option = argv[i];

        // Handle flags that don't require arguments
        if (option == "--serial") {
            numThreads = 1;
            useParallel = false;
            i++;
            continue;
        }
        if (option == "--local-search") {
            useLocalSearch = true;
            i++;
            continue;
        }
        if (option == "--2opt-only") {
            use3opt = false;
            i++;
            continue;
        }
        if (option == "--elitist") {
            useElitist = true;
            i++;
            continue;
        }

        // All other options require a value
        if (i + 1 >= argc) {
            std::cerr << "Error: Missing value for option " << argv[i] << std::endl;
            return 1;
        }

        std::string value = argv[i + 1];
        i += 2;

        try {
            if (option == "--ants") {
                numAnts = std::stoi(value);
                if (numAnts <= 0) {
                    std::cerr << "Error: Number of ants must be positive" << std::endl;
                    return 1;
                }
            } else if (option == "--iterations") {
                iterations = std::stoi(value);
                if (iterations <= 0) {
                    std::cerr << "Error: Number of iterations must be positive" << std::endl;
                    return 1;
                }
            } else if (option == "--alpha") {
                alpha = std::stod(value);
                if (alpha < 0.0) {
                    std::cerr << "Error: Alpha must be non-negative" << std::endl;
                    return 1;
                }
            } else if (option == "--beta") {
                beta = std::stod(value);
                if (beta < 0.0) {
                    std::cerr << "Error: Beta must be non-negative" << std::endl;
                    return 1;
                }
            } else if (option == "--rho") {
                rho = std::stod(value);
                if (rho < 0.0 || rho > 1.0) {
                    std::cerr << "Error: Rho must be between 0 and 1" << std::endl;
                    return 1;
                }
            } else if (option == "--Q") {
                Q = std::stod(value);
                if (Q <= 0.0) {
                    std::cerr << "Error: Q must be positive" << std::endl;
                    return 1;
                }
            } else if (option == "--threads") {
                numThreads = std::stoi(value);
                if (numThreads < 0) {
                    std::cerr << "Error: Number of threads must be non-negative" << std::endl;
                    return 1;
                }
                if (numThreads == 1) {
                    useParallel = false;
                }
            } else if (option == "--ls-mode") {
                if (value == "best" || value == "all" || value == "none") {
                    localSearchMode = value;
                } else {
                    std::cerr << "Error: --ls-mode must be 'best', 'all', or 'none'" << std::endl;
                    return 1;
                }
            } else if (option == "--elitist-weight") {
                elitistWeight = std::stod(value);
                if (elitistWeight < 0.0) {
                    std::cerr << "Error: Elitist weight must be non-negative" << std::endl;
                    return 1;
                }
            } else if (option == "--pheromone-mode") {
                if (value == "all" || value == "best-iteration" || value == "best-so-far" || value == "rank") {
                    pheromoneMode = value;
                } else {
                    std::cerr << "Error: --pheromone-mode must be 'all', 'best-iteration', 'best-so-far', or 'rank'" << std::endl;
                    return 1;
                }
            } else if (option == "--rank-size") {
                rankSize = std::stoi(value);
                if (rankSize < 0) {
                    std::cerr << "Error: Rank size must be non-negative" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: Unknown option " << option << std::endl;
                printUsage(argv[0]);
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid value for option " << option << ": " << value << std::endl;
            return 1;
        }
    }

    // Print header
    std::cout << "========================================\n";
    std::cout << "Ant Colony Optimization - TSP Solver\n";
    std::cout << "========================================\n\n";

    // Load TSP problem
    std::cout << "Loading TSP instance from: " << inputFile << std::endl;
    TSPLoader loader(inputFile);
    Graph graph = loader.loadGraph();

    if (!graph.isValid()) {
        std::cerr << "Error: Failed to load TSP instance from " << inputFile << std::endl;
        return 1;
    }

    std::cout << "Successfully loaded " << graph.getNumCities() << " cities\n\n";

    // If numAnts not specified, use one ant per city with distinct start positions
    bool useDistinctStartCities = false;
    if (numAnts == -1) {
        numAnts = graph.getNumCities();
        useDistinctStartCities = true;
    }

    // Print parameters
    std::cout << "Algorithm Parameters:\n";
    std::cout << "  Number of ants:       " << numAnts;
    if (useDistinctStartCities) {
        std::cout << " (each starting at different city)";
    }
    std::cout << "\n";
    std::cout << "  Iterations:           ";
    if (iterations < 0) {
        std::cout << "Until no improvement for 200 iterations\n";
    } else {
        std::cout << iterations << "\n";
    }
    std::cout << "  Alpha (pheromone):    " << alpha << "\n";
    std::cout << "  Beta (heuristic):     " << beta << "\n";
    std::cout << "  Rho (evaporation):    " << rho << "\n";
    std::cout << "  Q (deposit factor):   " << Q << "\n";
    std::cout << "  Threading:            ";
#ifdef _OPENMP
    if (!useParallel || numThreads == 1) {
        std::cout << "Serial (single-threaded)\n";
    } else if (numThreads == 0) {
        std::cout << omp_get_max_threads() << " threads (auto-detected)\n";
    } else {
        std::cout << numThreads << " threads\n";
    }
#else
    std::cout << "Serial (OpenMP not available)\n";
#endif
    std::cout << "  Local Search:         ";
    if (useLocalSearch) {
        std::cout << "Enabled (" << (use3opt ? "2-opt + 3-opt" : "2-opt only")
                  << ", mode: " << localSearchMode << ")\n";
    } else {
        std::cout << "Disabled\n";
    }
    std::cout << "  Elitist Strategy:     ";
    if (useElitist) {
        double effectiveWeight = (elitistWeight >= 0.0) ? elitistWeight : static_cast<double>(numAnts);
        std::cout << "Enabled (weight: " << effectiveWeight << ")\n";
    } else {
        std::cout << "Disabled\n";
    }
    std::cout << "  Pheromone Mode:       " << pheromoneMode;
    if (pheromoneMode == "rank") {
        int effectiveRankSize = (rankSize >= 0) ? rankSize : (numAnts / 2);
        std::cout << " (top " << effectiveRankSize << " ants)";
    }
    std::cout << "\n";
    std::cout << "\n";

    // Initialize and run ACO
    std::cout << "Running Ant Colony Optimization...\n";
    AntColony colony(graph, numAnts, alpha, beta, rho, Q, useDistinctStartCities);

    // Configure threading
    colony.setUseParallel(useParallel);
    colony.setNumThreads(numThreads);

    // Configure local search
    colony.setUseLocalSearch(useLocalSearch);
    colony.setUse3Opt(use3opt);
    colony.setLocalSearchMode(localSearchMode);

    // Configure elitist strategy
    colony.setUseElitist(useElitist);
    if (elitistWeight >= 0.0) {
        colony.setElitistWeight(elitistWeight);
    }
    colony.setPheromoneMode(pheromoneMode);
    if (rankSize >= 0) {
        colony.setRankSize(rankSize);
    }

    // Progress callback to show updates every 10 iterations
    int lastReportedIteration = 0;
    auto progressCallback = [&lastReportedIteration](int iteration, double bestDistance,
                                                      const std::vector<int>& /*bestTour*/,
                                                      const std::vector<double>& /*convergence*/) {
        // Report every 10 iterations
        if (iteration % 10 == 0 || iteration == 1) {
            std::cout << "  Iteration " << std::setw(5) << iteration
                      << " | Best distance: " << std::fixed << std::setprecision(2)
                      << bestDistance << "\n";
            std::cout.flush();
            lastReportedIteration = iteration;
        }
    };

    Tour bestTour = colony.solve(iterations, progressCallback);

    // Report final iteration if not already reported
    if (lastReportedIteration != static_cast<int>(colony.getConvergenceData().size())) {
        std::cout << "  Iteration " << std::setw(5) << colony.getConvergenceData().size()
                  << " | Best distance: " << std::fixed << std::setprecision(2)
                  << bestTour.getDistance() << "\n";
    }

    // Print results
    std::cout << "\n========================================\n";
    std::cout << "Results\n";
    std::cout << "========================================\n\n";

    std::cout << "Best tour distance: " << std::fixed << std::setprecision(2)
              << bestTour.getDistance() << "\n\n";

    std::cout << "Best tour sequence:\n";
    const auto& sequence = bestTour.getSequence();
    for (size_t i = 0; i < sequence.size(); ++i) {
        std::cout << sequence[i];
        if (i < sequence.size() - 1) {
            std::cout << " -> ";
        }
    }
    std::cout << " -> " << sequence[0] << " (return to start)\n\n";

    // Print convergence summary
    const auto& convergenceData = colony.getConvergenceData();
    if (!convergenceData.empty()) {
        std::cout << "Convergence Summary:\n";
        std::cout << "  First iteration best: " << std::fixed << std::setprecision(2)
                  << convergenceData.front() << "\n";
        std::cout << "  Final iteration best: " << std::fixed << std::setprecision(2)
                  << convergenceData.back() << "\n";
        std::cout << "  Overall best:         " << std::fixed << std::setprecision(2)
                  << bestTour.getDistance() << "\n";

        double improvement = convergenceData.front() - bestTour.getDistance();
        double improvementPercent = (improvement / convergenceData.front()) * 100.0;
        std::cout << "  Improvement:          " << std::fixed << std::setprecision(2)
                  << improvement << " (" << improvementPercent << "%)\n";
    }

    std::cout << "\n========================================\n";

    return 0;
}
