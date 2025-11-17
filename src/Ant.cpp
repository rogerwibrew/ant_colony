#include "Ant.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <stdexcept>

Ant::Ant(int startCity, int numCities)
    : currentCity_(startCity),
      visited_(numCities, false),
      tourLength_(0.0),
      numCities_(numCities) {
    tour_.reserve(numCities);
    tour_.push_back(startCity);
    visited_[startCity] = true;
}

void Ant::reset(int startCity) {
    currentCity_ = startCity;
    std::fill(visited_.begin(), visited_.end(), false);
    tour_.clear();
    tour_.push_back(startCity);
    visited_[startCity] = true;
    tourLength_ = 0.0;
}

int Ant::selectNextCity(const Graph& graph, const PheromoneMatrix& pheromones,
                       double alpha, double beta) {
    // Build list of unvisited cities
    std::vector<int> unvisited;
    for (int i = 0; i < numCities_; ++i) {
        if (!visited_[i]) {
            unvisited.push_back(i);
        }
    }

    // If no unvisited cities, return -1
    if (unvisited.empty()) {
        return -1;
    }

    // Calculate probabilities for each unvisited city
    std::vector<double> probabilities;
    probabilities.reserve(unvisited.size());
    double totalProbability = 0.0;

    for (int city : unvisited) {
        double pheromone = pheromones.getPheromone(currentCity_, city);
        double distance = graph.getDistance(currentCity_, city);

        // Avoid division by zero
        if (distance == 0.0) {
            distance = 1e-10;
        }

        double heuristic = 1.0 / distance;

        // Probability: pheromone^alpha * heuristic^beta
        double probability = std::pow(pheromone, alpha) * std::pow(heuristic, beta);

        probabilities.push_back(probability);
        totalProbability += probability;
    }

    // Handle edge case where all probabilities are 0
    if (totalProbability == 0.0) {
        // Select randomly - use thread_local for thread safety
        thread_local std::random_device rd;
        thread_local std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, unvisited.size() - 1);
        return unvisited[dist(gen)];
    }

    // Normalize probabilities
    for (double& prob : probabilities) {
        prob /= totalProbability;
    }

    // Roulette wheel selection - use thread_local for thread safety
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);
    double random = dist(gen);

    double cumulativeProbability = 0.0;
    for (size_t i = 0; i < unvisited.size(); ++i) {
        cumulativeProbability += probabilities[i];
        if (random <= cumulativeProbability) {
            return unvisited[i];
        }
    }

    // Should not reach here, but return last city as fallback
    return unvisited.back();
}

void Ant::visitCity(int city, const Graph& graph) {
    if (visited_[city]) {
        throw std::runtime_error("City already visited");
    }

    // Add distance from current city to new city
    tourLength_ += graph.getDistance(currentCity_, city);

    // Update state
    currentCity_ = city;
    visited_[city] = true;
    tour_.push_back(city);
}

bool Ant::hasVisitedAll() const {
    return tour_.size() == static_cast<size_t>(numCities_);
}

Tour Ant::completeTour(const Graph& graph) {
    if (!hasVisitedAll()) {
        throw std::runtime_error("Tour is not complete");
    }

    // Add distance back to starting city
    double returnDistance = graph.getDistance(currentCity_, tour_[0]);
    double totalDistance = tourLength_ + returnDistance;

    return Tour(tour_, totalDistance);
}

double Ant::calculateTourLength(const Graph& graph) const {
    if (tour_.empty()) {
        return 0.0;
    }

    double distance = 0.0;
    for (size_t i = 0; i < tour_.size() - 1; ++i) {
        distance += graph.getDistance(tour_[i], tour_[i + 1]);
    }

    // Add distance back to start if tour is complete
    if (hasVisitedAll()) {
        distance += graph.getDistance(tour_.back(), tour_[0]);
    }

    return distance;
}
