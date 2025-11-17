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

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input_file> [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --ants <n>       Number of ants (default: number of cities)\n";
    std::cout << "  --iterations <n> Number of iterations (default: until no improvement for 200 iterations)\n";
    std::cout << "  --alpha <f>      Pheromone importance (default: 1.0)\n";
    std::cout << "  --beta <f>       Heuristic importance (default: 2.0)\n";
    std::cout << "  --rho <f>        Evaporation rate (default: 0.5)\n";
    std::cout << "  --Q <f>          Pheromone deposit factor (default: 100.0)\n";
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

    // Parse command-line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    inputFile = argv[1];

    // Parse optional arguments
    for (int i = 2; i < argc; i += 2) {
        if (i + 1 >= argc) {
            std::cerr << "Error: Missing value for option " << argv[i] << std::endl;
            return 1;
        }

        std::string option = argv[i];
        std::string value = argv[i + 1];

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
    std::cout << "  Q (deposit factor):   " << Q << "\n\n";

    // Initialize and run ACO
    std::cout << "Running Ant Colony Optimization...\n";
    AntColony colony(graph, numAnts, alpha, beta, rho, Q, useDistinctStartCities);

    // Progress callback to show updates every 10 iterations
    int lastReportedIteration = 0;
    auto progressCallback = [&lastReportedIteration](int iteration, double bestDistance) {
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
