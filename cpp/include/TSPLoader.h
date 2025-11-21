/**
 * @file TSPLoader.h
 * @brief Loads TSP problem instances from files
 *
 * This class supports loading TSP problems from two file formats:
 * 1. Coordinate format: City ID, X, Y coordinates
 * 2. Distance matrix format: Precomputed distance matrix
 */

#ifndef TSPLOADER_H
#define TSPLOADER_H

#include "City.h"
#include "Graph.h"
#include <string>
#include <vector>

/**
 * @class TSPLoader
 * @brief Loads and parses TSP problem instances from files
 *
 * The loader can auto-detect file format or load specific formats directly.
 * Performs validation and error checking during loading.
 */
class TSPLoader {
public:
  /**
   * @brief Construct a TSPLoader for a specific file
   * @param filename Path to the TSP problem file
   *
   * Marked explicit to prevent implicit string-to-TSPLoader conversions.
   */
  explicit TSPLoader(const std::string &filename);

  /**
   * @brief Load graph from file with automatic format detection
   * @return Graph object containing cities and distances
   *
   * Returns empty Graph if loading fails.
   */
  Graph loadGraph();

  /**
   * @brief Load from coordinate format file
   * @param filename Path to coordinate format file
   * @return Graph constructed from city coordinates
   *
   * Expected format:
   * Line 1: Number of cities (n)
   * Lines 2 to n+1: City_ID X_coordinate Y_coordinate
   */
  static Graph loadFromCoordinates(const std::string &filename);

  /**
   * @brief Load from distance matrix format file
   * @param filename Path to distance matrix format file
   * @return Graph with synthetic coordinates approximating distances
   *
   * Expected format:
   * Line 1: Number of cities (n)
   * Lines 2 to n+1: n distance values per line (n×n matrix)
   *
   * Note: Since Graph requires City objects with coordinates, this method
   * generates synthetic coordinates. Exact distances may not be preserved.
   */
  static Graph loadFromDistanceMatrix(const std::string &filename);

  /**
   * @brief Load from TSPLIB format file
   * @param filename Path to TSPLIB format file
   * @return Graph constructed from TSPLIB data
   *
   * Supports TSPLIB format with NODE_COORD_SECTION.
   * Automatically detects and parses header information.
   */
  static Graph loadFromTSPLIB(const std::string &filename);

private:
  std::string filename_;  ///< Path to the file to load

  /**
   * @enum FileFormat
   * @brief Supported file formats for TSP problems
   */
  enum class FileFormat {
    COORDINATES,      ///< Format: id x y (one city per line)
    DISTANCE_MATRIX,  ///< Format: n×n distance matrix
    TSPLIB,           ///< Format: TSPLIB standard format
    UNKNOWN           ///< Could not determine format
  };

  /**
   * @brief Auto-detect the file format by analyzing content
   * @return FileFormat enum indicating the detected format
   *
   * Detection strategy:
   * - If second line has 3 fields → COORDINATES
   * - If second line has >3 fields → DISTANCE_MATRIX
   * - Otherwise → UNKNOWN
   */
  FileFormat detectFormat() const;
};

#endif // TSPLOADER_H
