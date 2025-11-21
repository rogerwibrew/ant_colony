/**
 * @file Tour.h
 * @brief Represents a complete tour (solution) visiting all cities
 *
 * This class stores a complete tour as a sequence of city IDs and tracks
 * the total distance. It provides validation to ensure the tour is valid
 * (visits all cities exactly once).
 */

#ifndef TOUR_H
#define TOUR_H

#include <vector>

/**
 * @class Tour
 * @brief A solution to the TSP - a sequence of cities and total distance
 *
 * A tour represents a complete path through all cities, returning to the
 * start. The tour stores the city sequence (as city IDs) and the total
 * distance traveled.
 */
class Tour {
public:
    /**
     * @brief Default constructor creating an empty/invalid tour
     */
    Tour();

    /**
     * @brief Construct a tour with a given sequence and distance
     * @param sequence Ordered list of city IDs to visit
     * @param distance Total tour length (including return to start)
     */
    Tour(const std::vector<int>& sequence, double distance);

    /**
     * @brief Set the tour data
     * @param sequence Ordered list of city IDs to visit
     * @param distance Total tour length
     */
    void setTour(const std::vector<int>& sequence, double distance);

    /**
     * @brief Get the total distance of the tour
     * @return double Total tour length
     */
    double getDistance() const;

    /**
     * @brief Get the sequence of cities in the tour
     * @return const std::vector<int>& City ID sequence
     */
    const std::vector<int>& getSequence() const;

    /**
     * @brief Validate the tour against the number of cities
     * @param numCities Expected number of cities in the problem
     * @return true if tour visits all cities exactly once, false otherwise
     *
     * Checks that:
     * - Tour length matches numCities
     * - All city IDs are in valid range [0, numCities-1]
     * - No city is visited more than once
     */
    bool validate(int numCities) const;

private:
    std::vector<int> citySequence_;  ///< Ordered list of city IDs
    double totalDistance_;           ///< Total tour length
    bool isValid_;                   ///< Whether tour has been set with valid data
};

#endif // TOUR_H
