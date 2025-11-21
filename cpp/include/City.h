/**
 * @file City.h
 * @brief Represents a single city in the TSP instance
 *
 * This class stores a city's unique identifier and 2D coordinates,
 * providing methods to calculate distances between cities.
 */

#ifndef CITY_H
#define CITY_H

#include <cmath>

/**
 * @class City
 * @brief A city in the Travelling Salesman Problem with 2D coordinates
 *
 * Each city has a unique ID and (x, y) coordinates in a 2D plane.
 * Distances between cities are calculated using the Euclidean distance formula.
 */
class City {
public:
    /**
     * @brief Construct a new City object
     * @param id Unique identifier for the city
     * @param x X-coordinate in 2D space
     * @param y Y-coordinate in 2D space
     */
    City(int id, double x, double y);

    /**
     * @brief Calculate Euclidean distance to another city
     * @param other The destination city
     * @return double Distance between this city and the other city
     *
     * Uses the formula: sqrt((x2-x1)^2 + (y2-y1)^2)
     */
    double distanceTo(const City& other) const;

    /**
     * @brief Get the city's unique identifier
     * @return int The city ID
     */
    int getId() const;

    /**
     * @brief Get the X-coordinate
     * @return double The X position
     */
    double getX() const;

    /**
     * @brief Get the Y-coordinate
     * @return double The Y position
     */
    double getY() const;

private:
    int id_;     ///< Unique identifier for this city
    double x_;   ///< X-coordinate in 2D space
    double y_;   ///< Y-coordinate in 2D space
};

#endif // CITY_H
