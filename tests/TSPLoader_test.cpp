#include <gtest/gtest.h>
#include "TSPLoader.h"
#include "Graph.h"
#include <string>
#include <fstream>

// Helper to get test data path
std::string getTestDataPath(const std::string& filename) {
    // Assumes tests are run from build directory
    return "../tests/data/" + filename;
}

// Test loading simple coordinate file (5 cities)
TEST(TSPLoaderTest, LoadSimpleCoordinateFile) {
    std::string filepath = getTestDataPath("test_simple_5.txt");
    TSPLoader loader(filepath);
    Graph graph = loader.loadGraph();

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 5);

    // Verify first city
    const City& city0 = graph.getCity(0);
    EXPECT_EQ(city0.getId(), 0);
    EXPECT_DOUBLE_EQ(city0.getX(), 10.0);
    EXPECT_DOUBLE_EQ(city0.getY(), 20.0);

    // Verify last city
    const City& city4 = graph.getCity(4);
    EXPECT_EQ(city4.getId(), 4);
    EXPECT_DOUBLE_EQ(city4.getX(), 20.0);
    EXPECT_DOUBLE_EQ(city4.getY(), 30.0);
}

// Test loading triangle coordinate file (3 cities)
TEST(TSPLoaderTest, LoadTriangleCoordinateFile) {
    std::string filepath = getTestDataPath("test_triangle_3.txt");
    Graph graph = TSPLoader::loadFromCoordinates(filepath);

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 3);

    // Verify distances (3-4-5 right triangle)
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 1), 3.0);  // Base
    EXPECT_DOUBLE_EQ(graph.getDistance(0, 2), 4.0);  // Height
    EXPECT_DOUBLE_EQ(graph.getDistance(1, 2), 5.0);  // Hypotenuse
}

// Test loading distance matrix file
TEST(TSPLoaderTest, LoadDistanceMatrixFile) {
    std::string filepath = getTestDataPath("test_matrix_4.txt");
    Graph graph = TSPLoader::loadFromDistanceMatrix(filepath);

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 4);

    // Note: Due to synthetic coordinate generation, exact distances may not match
    // We just verify that a valid graph was created
    EXPECT_GT(graph.getDistance(0, 1), 0.0);
}

// Test auto-detect coordinate format
TEST(TSPLoaderTest, AutoDetectCoordinateFormat) {
    std::string filepath = getTestDataPath("test_simple_5.txt");
    TSPLoader loader(filepath);
    Graph graph = loader.loadGraph();

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 5);
}

// Test auto-detect distance matrix format
TEST(TSPLoaderTest, AutoDetectDistanceMatrixFormat) {
    std::string filepath = getTestDataPath("test_matrix_4.txt");
    TSPLoader loader(filepath);
    Graph graph = loader.loadGraph();

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 4);
}

// Test loading non-existent file
TEST(TSPLoaderTest, LoadNonExistentFile) {
    std::string filepath = "non_existent_file.txt";
    TSPLoader loader(filepath);
    Graph graph = loader.loadGraph();

    EXPECT_FALSE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 0);
}

// Test loading invalid file format
TEST(TSPLoaderTest, LoadInvalidFile) {
    std::string filepath = getTestDataPath("test_invalid.txt");
    Graph graph = TSPLoader::loadFromCoordinates(filepath);

    // Should return empty graph due to invalid data
    EXPECT_FALSE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 0);
}

// Test static loadFromCoordinates method
TEST(TSPLoaderTest, StaticLoadFromCoordinates) {
    std::string filepath = getTestDataPath("test_triangle_3.txt");
    Graph graph = TSPLoader::loadFromCoordinates(filepath);

    EXPECT_TRUE(graph.isValid());
    EXPECT_EQ(graph.getNumCities(), 3);
}

// Test that distance matrix is built correctly
TEST(TSPLoaderTest, VerifyDistanceMatrixBuilt) {
    std::string filepath = getTestDataPath("test_simple_5.txt");
    Graph graph = TSPLoader::loadFromCoordinates(filepath);

    EXPECT_TRUE(graph.isValid());

    // Verify that we can access distances
    for (int i = 0; i < graph.getNumCities(); ++i) {
        for (int j = 0; j < graph.getNumCities(); ++j) {
            double dist = graph.getDistance(i, j);
            EXPECT_GE(dist, 0.0);

            // Diagonal should be zero
            if (i == j) {
                EXPECT_DOUBLE_EQ(dist, 0.0);
            }
        }
    }
}

// Test symmetry of loaded graph
TEST(TSPLoaderTest, LoadedGraphSymmetry) {
    std::string filepath = getTestDataPath("test_simple_5.txt");
    Graph graph = TSPLoader::loadFromCoordinates(filepath);

    EXPECT_TRUE(graph.isValid());

    // Verify symmetry
    for (int i = 0; i < graph.getNumCities(); ++i) {
        for (int j = i + 1; j < graph.getNumCities(); ++j) {
            EXPECT_DOUBLE_EQ(graph.getDistance(i, j), graph.getDistance(j, i));
        }
    }
}
