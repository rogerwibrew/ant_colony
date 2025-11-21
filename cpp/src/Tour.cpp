/**
 * @file Tour.cpp
 * @brief Implementation of the Tour class
 */

#include "Tour.h"
#include <algorithm>
#include <unordered_set>

Tour::Tour()
    : citySequence_(),
      totalDistance_(0.0),
      isValid_(false) {
}

Tour::Tour(const std::vector<int>& sequence, double distance)
    : citySequence_(sequence),
      totalDistance_(distance),
      isValid_(true) {
}

void Tour::setTour(const std::vector<int>& sequence, double distance) {
    citySequence_ = sequence;
    totalDistance_ = distance;
    isValid_ = true;
}

double Tour::getDistance() const {
    return totalDistance_;
}

const std::vector<int>& Tour::getSequence() const {
    return citySequence_;
}

bool Tour::validate(int numCities) const {
    // Empty tour is invalid
    if (citySequence_.empty()) {
        return false;
    }

    // Tour must have exactly numCities cities
    if (static_cast<int>(citySequence_.size()) != numCities) {
        return false;
    }

    // Use unordered_set to check for duplicates and valid range
    std::unordered_set<int> visitedCities;

    for (int cityId : citySequence_) {
        // Check if city ID is in valid range [0, numCities-1]
        if (cityId < 0 || cityId >= numCities) {
            return false;
        }

        // Check for duplicate cities
        if (visitedCities.count(cityId) > 0) {
            return false;
        }

        visitedCities.insert(cityId);
    }

    return true;
}
