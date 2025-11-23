/**
 * @file LocalSearch.h
 * @brief Local search improvement algorithms for TSP tours
 *
 * This class provides static methods for improving TSP tours using
 * local search heuristics like 2-opt and 3-opt. These methods can
 * significantly improve tour quality (typically 5-15% better) with
 * moderate computational cost.
 */

#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "Tour.h"
#include "Graph.h"

/**
 * @class LocalSearch
 * @brief Static utility class for local search optimization
 *
 * Provides 2-opt and 3-opt algorithms for post-processing tours.
 * These algorithms iteratively improve tours by swapping edges until
 * a local optimum is reached.
 */
class LocalSearch {
public:
    /**
     * @brief Improve a tour using 2-opt edge swapping
     * @param tour The tour to improve (will be modified in-place)
     * @param graph The graph containing distance information
     * @return true if any improvement was made, false if already at local optimum
     *
     * 2-opt works by:
     * 1. For each pair of edges (i,i+1) and (j,j+1) in the tour
     * 2. Try reversing the segment between them
     * 3. If this reduces tour length, keep the change
     * 4. Repeat until no improvements found
     *
     * Time complexity: O(n²) per iteration, typically converges in few iterations
     */
    static bool twoOpt(Tour& tour, const Graph& graph);

    /**
     * @brief Improve a tour using 3-opt edge swapping
     * @param tour The tour to improve (will be modified in-place)
     * @param graph The graph containing distance information
     * @return true if any improvement was made, false if already at local optimum
     *
     * 3-opt works by:
     * 1. Remove 3 edges from the tour
     * 2. Try all 7 possible reconnection patterns
     * 3. Keep the best reconnection if it improves tour length
     * 4. Repeat until no improvements found
     *
     * Time complexity: O(n³) per iteration, higher quality than 2-opt
     * Note: More expensive than 2-opt, use selectively
     */
    static bool threeOpt(Tour& tour, const Graph& graph);

    /**
     * @brief Apply both 2-opt and 3-opt in sequence
     * @param tour The tour to improve (will be modified in-place)
     * @param graph The graph containing distance information
     * @param use3opt If false, only apply 2-opt (default: true)
     * @return true if any improvement was made
     *
     * Typically runs 2-opt first (fast), then optionally 3-opt (slower but better).
     * This provides a good balance between speed and solution quality.
     */
    static bool improve(Tour& tour, const Graph& graph, bool use3opt = true);

private:
    /**
     * @brief Calculate the change in tour distance from a 2-opt swap
     * @param sequence Current tour sequence
     * @param graph Graph with distance information
     * @param i First edge start index
     * @param j Second edge start index
     * @return Negative value means improvement, positive means worse
     *
     * Computes delta = new_distance - old_distance without reconstructing tour
     */
    static double calculate2OptDelta(const std::vector<int>& sequence,
                                     const Graph& graph, int i, int j);

    /**
     * @brief Reverse a segment of the tour in-place
     * @param sequence Tour sequence to modify
     * @param i Start index of segment (inclusive)
     * @param j End index of segment (inclusive)
     *
     * Helper function for applying 2-opt swaps
     */
    static void reverseTourSegment(std::vector<int>& sequence, int i, int j);

    /**
     * @brief Calculate total tour distance given a sequence
     * @param sequence Tour sequence (city IDs)
     * @param graph Graph with distance information
     * @return Total tour distance including return to start
     */
    static double calculateTourDistance(const std::vector<int>& sequence,
                                        const Graph& graph);
};

#endif // LOCALSEARCH_H
