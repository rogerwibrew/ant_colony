/**
 * @file TSPLoader.cpp
 * @brief Implementation of TSP file loading and parsing
 */

#include "TSPLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <sys/stat.h>

// Helper function to check if a file exists
static bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Helper function to find the file in common locations
static std::string findFile(const std::string& filename) {
    // List of paths to search (in order of preference)
    std::vector<std::string> searchPaths = {
        filename,                    // Current directory or absolute path
        "data/" + filename,          // data/ subdirectory
        "../data/" + filename,       // data/ from build directory
        "../../data/" + filename     // data/ from build/bin directory
    };

    for (const auto& path : searchPaths) {
        if (fileExists(path)) {
            return path;
        }
    }

    // File not found in any location
    return filename; // Return original name to preserve error messages
}

// Constructor - stores filename for later loading
TSPLoader::TSPLoader(const std::string& filename) : filename_(findFile(filename)) {}

/**
 * Load a graph by automatically detecting the file format.
 * This is the recommended method for loading files when the format is unknown.
 *
 * @return Graph object on success, empty Graph on failure
 */
Graph TSPLoader::loadGraph() {
    // First, analyze the file to determine its format
    FileFormat format = detectFormat();

    // Route to appropriate loader based on detected format
    if (format == FileFormat::COORDINATES) {
        return loadFromCoordinates(filename_);
    } else if (format == FileFormat::DISTANCE_MATRIX) {
        return loadFromDistanceMatrix(filename_);
    } else if (format == FileFormat::TSPLIB) {
        return loadFromTSPLIB(filename_);
    } else {
        // Format detection failed or file unreadable
        std::cerr << "Error: Unknown file format or unable to read file: " << filename_ << std::endl;
        return Graph(); // Return empty graph
    }
}

/**
 * Detect the file format by examining the first few lines.
 * Uses heuristics based on the number of fields in the second data line.
 *
 * Format detection logic:
 * - TSPLIB format: First line contains "NAME:" or "TYPE:"
 * - 3 fields in second line → COORDINATES format (id, x, y)
 * - >3 fields in second line → DISTANCE_MATRIX format (row of matrix)
 * - Otherwise → Default to COORDINATES or UNKNOWN
 *
 * @return FileFormat enum indicating detected format
 */
TSPLoader::FileFormat TSPLoader::detectFormat() const {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename_ << std::endl;
        return FileFormat::UNKNOWN;
    }

    // Read first two lines for analysis
    std::string firstLine, secondLine;
    std::getline(file, firstLine);  // Usually contains number of cities
    std::getline(file, secondLine); // Data line - format depends on file type

    // Validate that we have content to analyze
    if (firstLine.empty() || secondLine.empty()) {
        return FileFormat::UNKNOWN;
    }

    // Check for TSPLIB format (has "NAME", "TYPE", "DIMENSION" keywords in first few lines)
    if (firstLine.find("NAME") != std::string::npos ||
        firstLine.find("TYPE") != std::string::npos ||
        secondLine.find("TYPE") != std::string::npos ||
        secondLine.find("COMMENT") != std::string::npos ||
        secondLine.find("DIMENSION") != std::string::npos) {
        return FileFormat::TSPLIB;
    }

    // Parse second line into fields (whitespace-separated)
    std::istringstream ss(secondLine);
    std::vector<std::string> fields;
    std::string field;
    while (ss >> field) {
        fields.push_back(field);
    }

    // Heuristic: count fields to determine format
    // Coordinate format: ID X Y (3 fields)
    // Distance matrix format: d1 d2 d3 ... dn (n fields where n > 3)
    if (fields.size() == 3) {
        return FileFormat::COORDINATES;
    } else if (fields.size() > 3) {
        return FileFormat::DISTANCE_MATRIX;
    } else {
        // Ambiguous - default to coordinates for 1-2 fields
        return FileFormat::COORDINATES;
    }
}

/**
 * Load a TSP problem from coordinate format file.
 *
 * Expected file format:
 * Line 1: n (number of cities)
 * Lines 2 to n+1: city_id x_coordinate y_coordinate
 *
 * Example:
 * 5
 * 0 10.0 20.0
 * 1 15.0 25.0
 * 2 30.0 40.0
 * 3 50.0 10.0
 * 4 20.0 30.0
 *
 * Performs extensive validation:
 * - Checks file can be opened
 * - Validates number of cities is positive
 * - Ensures all coordinates are finite numbers (not NaN or infinity)
 * - Verifies correct number of cities were read
 *
 * @param filename Path to the coordinate format file
 * @return Graph constructed from the coordinates, or empty Graph on error
 */
Graph TSPLoader::loadFromCoordinates(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return Graph();
    }

    // Read number of cities from first line
    int numCities;
    file >> numCities;

    if (numCities <= 0) {
        std::cerr << "Error: Invalid number of cities: " << numCities << std::endl;
        return Graph();
    }

    // Pre-allocate vector for efficiency
    std::vector<City> cities;
    cities.reserve(numCities);

    // Read each city's data
    for (int i = 0; i < numCities; ++i) {
        int id;
        double x, y;

        // Attempt to read three values: ID, X, Y
        if (!(file >> id >> x >> y)) {
            std::cerr << "Error: Failed to read city data at line " << (i + 2) << std::endl;
            return Graph();
        }

        // Validate that coordinates are finite numbers (not NaN or infinity)
        if (!std::isfinite(x) || !std::isfinite(y)) {
            std::cerr << "Error: Invalid coordinates for city " << id << std::endl;
            return Graph();
        }

        // Create and add city to vector
        cities.push_back(City(id, x, y));
    }

    // Verify we read the expected number of cities
    if (cities.size() != static_cast<size_t>(numCities)) {
        std::cerr << "Error: Expected " << numCities << " cities, but read " << cities.size() << std::endl;
        return Graph();
    }

    // Construct and return graph (distance matrix will be computed automatically)
    return Graph(cities);
}

/**
 * Load a TSP problem from distance matrix format file.
 *
 * Expected file format:
 * Line 1: n (number of cities)
 * Lines 2 to n+1: n distance values per line (forming an n×n matrix)
 *
 * Example for 3 cities:
 * 3
 * 0.0 10.5 20.3
 * 10.5 0.0 15.8
 * 20.3 15.8 0.0
 *
 * IMPORTANT LIMITATION:
 * Since the Graph class requires City objects with (x,y) coordinates,
 * this method generates SYNTHETIC coordinates that approximate the distances.
 * The actual distances in the resulting graph may not perfectly match the
 * input distance matrix.
 *
 * A proper implementation would use Multi-Dimensional Scaling (MDS) to
 * reconstruct coordinates that preserve distances as much as possible.
 * This simplified version uses a basic circular placement strategy.
 *
 * Validation performed:
 * - File accessibility
 * - Positive number of cities
 * - All distances are finite and non-negative
 * - Matrix symmetry (warning if not symmetric)
 *
 * @param filename Path to the distance matrix file
 * @return Graph with synthetic coordinates, or empty Graph on error
 */
Graph TSPLoader::loadFromDistanceMatrix(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return Graph();
    }

    // Read number of cities
    int numCities;
    file >> numCities;

    if (numCities <= 0) {
        std::cerr << "Error: Invalid number of cities: " << numCities << std::endl;
        return Graph();
    }

    // Allocate n×n distance matrix
    std::vector<std::vector<double>> distMatrix(numCities, std::vector<double>(numCities));

    // Read all distance values
    for (int i = 0; i < numCities; ++i) {
        for (int j = 0; j < numCities; ++j) {
            if (!(file >> distMatrix[i][j])) {
                std::cerr << "Error: Failed to read distance matrix at position [" << i << "][" << j << "]" << std::endl;
                return Graph();
            }

            // Validate: distances must be finite and non-negative
            if (!std::isfinite(distMatrix[i][j]) || distMatrix[i][j] < 0.0) {
                std::cerr << "Error: Invalid distance value at [" << i << "][" << j << "]" << std::endl;
                return Graph();
            }
        }
    }

    // Verify matrix symmetry (expected for undirected TSP)
    const double EPSILON = 1e-6;  // Tolerance for floating-point comparison
    for (int i = 0; i < numCities; ++i) {
        for (int j = i + 1; j < numCities; ++j) {
            if (std::abs(distMatrix[i][j] - distMatrix[j][i]) > EPSILON) {
                std::cerr << "Warning: Distance matrix is not symmetric at [" << i << "][" << j << "]" << std::endl;
            }
        }
    }

    // Generate synthetic city coordinates that approximate the distances
    // This is a simplified approach - proper MDS would be better
    std::vector<City> cities;
    cities.reserve(numCities);

    // Special cases for first two cities
    if (numCities >= 1) {
        // Place first city at origin
        cities.push_back(City(0, 0.0, 0.0));
    }
    if (numCities >= 2) {
        // Place second city on positive x-axis at distance d[0][1]
        cities.push_back(City(1, distMatrix[0][1], 0.0));
    }

    // For remaining cities, use circular placement around origin
    // Radius based on distance from first city
    // This is a crude approximation and won't preserve exact distances
    for (int i = 2; i < numCities; ++i) {
        double angle = 2.0 * M_PI * i / numCities;  // Evenly spaced angles
        double radius = distMatrix[0][i];           // Distance from city 0
        double x = radius * std::cos(angle);
        double y = radius * std::sin(angle);
        cities.push_back(City(i, x, y));
    }

    // Warning: distances in the resulting graph will be computed from these
    // synthetic coordinates and may differ from the input distance matrix
    std::cerr << "Note: Distance matrix loaded. Synthetic coordinates generated (may not preserve exact distances)." << std::endl;

    return Graph(cities);
}

/**
 * Load a TSP problem from TSPLIB format file.
 *
 * Supports TSPLIB files with NODE_COORD_SECTION and EUC_2D edge weight type.
 * The TSPLIB format includes header information followed by coordinate data.
 *
 * Expected structure:
 * NAME: <instance name>
 * TYPE: TSP
 * DIMENSION: <number of cities>
 * EDGE_WEIGHT_TYPE: EUC_2D (or other types)
 * NODE_COORD_SECTION
 * <city_id> <x> <y>
 * ...
 * EOF
 *
 * @param filename Path to the TSPLIB format file
 * @return Graph constructed from the TSPLIB data
 */
Graph TSPLoader::loadFromTSPLIB(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return Graph();
    }

    std::string line;
    int dimension = 0;
    bool inCoordSection = false;
    std::vector<City> cities;

    // Read file line by line
    while (std::getline(file, line)) {
        // Trim whitespace from line
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Check for EOF marker
        if (line == "EOF") {
            break;
        }

        // Parse header fields
        if (line.find("DIMENSION") != std::string::npos) {
            // Extract dimension value (handles both "DIMENSION:" and "DIMENSION :")
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::istringstream ss(line.substr(colonPos + 1));
                ss >> dimension;
                cities.reserve(dimension);
            }
        } else if (line == "NODE_COORD_SECTION") {
            inCoordSection = true;
            continue;
        } else if (line.find("EDGE_WEIGHT_SECTION") != std::string::npos ||
                   line.find("DISPLAY_DATA_SECTION") != std::string::npos ||
                   line.find("TOUR_SECTION") != std::string::npos) {
            // Reached a different section, stop reading coordinates
            break;
        }

        // Parse coordinate data if in the NODE_COORD_SECTION
        if (inCoordSection) {
            std::istringstream ss(line);
            int id;
            double x, y;

            if (ss >> id >> x >> y) {
                // Validate coordinates
                if (!std::isfinite(x) || !std::isfinite(y)) {
                    std::cerr << "Error: Invalid coordinates for city " << id << std::endl;
                    return Graph();
                }

                // TSPLIB cities are 1-indexed, convert to 0-indexed
                cities.push_back(City(id - 1, x, y));
            }
        }
    }

    // Validate that we read some cities
    if (cities.empty()) {
        std::cerr << "Error: No cities found in TSPLIB file" << std::endl;
        return Graph();
    }

    // Validate dimension matches if it was specified
    if (dimension > 0 && cities.size() != static_cast<size_t>(dimension)) {
        std::cerr << "Warning: Expected " << dimension << " cities, but read "
                  << cities.size() << std::endl;
    }

    return Graph(cities);
}
