/**
 * @file LocalSearch.cpp
 * @brief Implementation of local search algorithms
 */

#include "LocalSearch.h"
#include <algorithm>
#include <limits>

bool LocalSearch::twoOpt(Tour& tour, const Graph& graph) {
    std::vector<int> sequence = tour.getSequence();
    int n = static_cast<int>(sequence.size());

    if (n < 4) {
        // Too small for 2-opt (need at least 4 cities to swap 2 edges)
        return false;
    }

    bool improved = true;
    bool anyImprovement = false;

    // Keep trying until no more improvements found
    while (improved) {
        improved = false;

        // Try all pairs of edges (i,i+1) and (j,j+1)
        for (int i = 0; i < n - 2; ++i) {
            for (int j = i + 2; j < n; ++j) {
                // Skip if j is the last city and i is the first (already connected)
                if (i == 0 && j == n - 1) {
                    continue;
                }

                // Calculate improvement delta
                double delta = calculate2OptDelta(sequence, graph, i, j);

                // If improvement found, apply it
                if (delta < -1e-9) {  // Use epsilon for floating point comparison
                    reverseTourSegment(sequence, i + 1, j);
                    improved = true;
                    anyImprovement = true;
                    // Don't break - continue searching for more improvements in this iteration
                }
            }
        }
    }

    // Update tour if any improvements were made
    if (anyImprovement) {
        double newDistance = calculateTourDistance(sequence, graph);
        tour.setTour(sequence, newDistance);
    }

    return anyImprovement;
}

double LocalSearch::calculate2OptDelta(const std::vector<int>& sequence,
                                       const Graph& graph, int i, int j) {
    int n = static_cast<int>(sequence.size());

    // Current edges: (i, i+1) and (j, j+1)
    int city_i = sequence[i];
    int city_i_plus_1 = sequence[i + 1];
    int city_j = sequence[j];
    int city_j_plus_1 = sequence[(j + 1) % n];  // Wrap around for last city

    // Old distance: i->i+1 and j->j+1
    double oldDistance = graph.getDistance(city_i, city_i_plus_1) +
                        graph.getDistance(city_j, city_j_plus_1);

    // New distance after reversing segment: i->j and i+1->j+1
    double newDistance = graph.getDistance(city_i, city_j) +
                        graph.getDistance(city_i_plus_1, city_j_plus_1);

    return newDistance - oldDistance;
}

void LocalSearch::reverseTourSegment(std::vector<int>& sequence, int i, int j) {
    while (i < j) {
        std::swap(sequence[i], sequence[j]);
        ++i;
        --j;
    }
}

double LocalSearch::calculateTourDistance(const std::vector<int>& sequence,
                                          const Graph& graph) {
    double totalDistance = 0.0;
    int n = static_cast<int>(sequence.size());

    for (int i = 0; i < n; ++i) {
        int fromCity = sequence[i];
        int toCity = sequence[(i + 1) % n];  // Wrap around to first city
        totalDistance += graph.getDistance(fromCity, toCity);
    }

    return totalDistance;
}

bool LocalSearch::threeOpt(Tour& tour, const Graph& graph) {
    std::vector<int> sequence = tour.getSequence();
    int n = static_cast<int>(sequence.size());

    if (n < 6) {
        // Too small for 3-opt (need at least 6 cities for meaningful 3-edge swaps)
        return false;
    }

    bool improved = true;
    bool anyImprovement = false;

    while (improved) {
        improved = false;
        double bestDelta = 0.0;
        int best_i = -1, best_j = -1, best_k = -1;
        int bestCase = -1;

        // Try all combinations of 3 edges
        for (int i = 0; i < n - 4; ++i) {
            for (int j = i + 2; j < n - 2; ++j) {
                for (int k = j + 2; k < n; ++k) {
                    // Skip if k wraps to include i
                    if (i == 0 && k == n - 1) {
                        continue;
                    }

                    // Current edges: (i,i+1), (j,j+1), (k,k+1)
                    int city_i = sequence[i];
                    int city_i1 = sequence[i + 1];
                    int city_j = sequence[j];
                    int city_j1 = sequence[j + 1];
                    int city_k = sequence[k];
                    int city_k1 = sequence[(k + 1) % n];

                    // Current distance of the 3 edges
                    double oldDist = graph.getDistance(city_i, city_i1) +
                                    graph.getDistance(city_j, city_j1) +
                                    graph.getDistance(city_k, city_k1);

                    // Try all 7 possible reconnection patterns (case 0 is original)
                    // Case 1: Reverse segment (i+1, j)
                    double case1 = graph.getDistance(city_i, city_j) +
                                  graph.getDistance(city_i1, city_j1) +
                                  graph.getDistance(city_k, city_k1);

                    // Case 2: Reverse segment (j+1, k)
                    double case2 = graph.getDistance(city_i, city_i1) +
                                  graph.getDistance(city_j, city_k) +
                                  graph.getDistance(city_j1, city_k1);

                    // Case 3: Reverse both segments
                    double case3 = graph.getDistance(city_i, city_j) +
                                  graph.getDistance(city_i1, city_k) +
                                  graph.getDistance(city_j1, city_k1);

                    // Case 4: Swap segments (i+1,j) and (j+1,k)
                    double case4 = graph.getDistance(city_i, city_j1) +
                                  graph.getDistance(city_k, city_i1) +
                                  graph.getDistance(city_j, city_k1);

                    // Find best case
                    double cases[] = {case1, case2, case3, case4};
                    for (int c = 0; c < 4; ++c) {
                        double delta = cases[c] - oldDist;
                        if (delta < bestDelta - 1e-9) {  // Epsilon for floating point
                            bestDelta = delta;
                            best_i = i;
                            best_j = j;
                            best_k = k;
                            bestCase = c + 1;
                        }
                    }
                }
            }
        }

        // Apply best improvement if found
        if (bestCase > 0) {
            improved = true;
            anyImprovement = true;

            // Apply the transformation based on best case
            std::vector<int> newSequence;

            switch (bestCase) {
                case 1:  // Reverse (i+1, j)
                    newSequence.insert(newSequence.end(), sequence.begin(), sequence.begin() + best_i + 1);
                    newSequence.insert(newSequence.end(), sequence.rbegin() + (n - best_j - 1),
                                      sequence.rbegin() + (n - best_i - 1));
                    newSequence.insert(newSequence.end(), sequence.begin() + best_j + 1, sequence.end());
                    break;

                case 2:  // Reverse (j+1, k)
                    newSequence.insert(newSequence.end(), sequence.begin(), sequence.begin() + best_j + 1);
                    newSequence.insert(newSequence.end(), sequence.rbegin() + (n - best_k - 1),
                                      sequence.rbegin() + (n - best_j - 1));
                    newSequence.insert(newSequence.end(), sequence.begin() + best_k + 1, sequence.end());
                    break;

                case 3:  // Reverse both segments
                    newSequence.insert(newSequence.end(), sequence.begin(), sequence.begin() + best_i + 1);
                    newSequence.insert(newSequence.end(), sequence.rbegin() + (n - best_j - 1),
                                      sequence.rbegin() + (n - best_i - 1));
                    newSequence.insert(newSequence.end(), sequence.rbegin() + (n - best_k - 1),
                                      sequence.rbegin() + (n - best_j - 1));
                    newSequence.insert(newSequence.end(), sequence.begin() + best_k + 1, sequence.end());
                    break;

                case 4:  // Swap segments
                    newSequence.insert(newSequence.end(), sequence.begin(), sequence.begin() + best_i + 1);
                    newSequence.insert(newSequence.end(), sequence.begin() + best_j + 1, sequence.begin() + best_k + 1);
                    newSequence.insert(newSequence.end(), sequence.begin() + best_i + 1, sequence.begin() + best_j + 1);
                    newSequence.insert(newSequence.end(), sequence.begin() + best_k + 1, sequence.end());
                    break;
            }

            sequence = newSequence;
        }
    }

    // Update tour if any improvements were made
    if (anyImprovement) {
        double newDistance = calculateTourDistance(sequence, graph);
        tour.setTour(sequence, newDistance);
    }

    return anyImprovement;
}

bool LocalSearch::improve(Tour& tour, const Graph& graph, bool use3opt) {
    bool improved = false;

    // Apply 2-opt first (fast and effective)
    if (twoOpt(tour, graph)) {
        improved = true;
    }

    // Optionally apply 3-opt for additional refinement
    if (use3opt) {
        if (threeOpt(tour, graph)) {
            improved = true;
        }
    }

    return improved;
}
