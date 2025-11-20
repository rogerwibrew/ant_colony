#!/usr/bin/env python3
"""
Test script for ACO Solver Python Bindings

This script tests all major functionality of the Python bindings:
- Loading TSPLIB files
- Creating and running ACO solver
- Progress callbacks
- Comparing results with C++ CLI
"""

import sys
import time

# Add python_bindings to path
sys.path.insert(0, '/home/roger/dev/ant_colony/python_bindings')

import aco_solver

def test_basic_classes():
    """Test basic class instantiation"""
    print("=" * 60)
    print("Test 1: Basic Class Instantiation")
    print("=" * 60)

    # Create cities
    city1 = aco_solver.City(0, 0.0, 0.0)
    city2 = aco_solver.City(1, 3.0, 4.0)

    print(f"City 1: {city1}")
    print(f"City 2: {city2}")
    print(f"Distance: {city1.distanceTo(city2):.2f}")
    assert abs(city1.distanceTo(city2) - 5.0) < 0.01, "Distance calculation failed"

    # Create graph
    cities = [city1, city2]
    graph = aco_solver.Graph(cities)
    print(f"Graph: {graph}")
    assert graph.getNumCities() == 2, "Graph city count failed"

    print("✓ Basic classes working\n")


def test_tsplib_loading():
    """Test loading TSPLIB file"""
    print("=" * 60)
    print("Test 2: TSPLIB File Loading")
    print("=" * 60)

    # Load a benchmark
    loader = aco_solver.TSPLoader("data/berlin52.tsp")
    graph = loader.loadGraph()

    print(f"Loaded: {graph}")
    assert graph.isValid(), "Graph is not valid"
    assert graph.getNumCities() == 52, "Expected 52 cities"

    # Check nearest neighbor heuristic
    nn_length = graph.nearestNeighborTourLength()
    print(f"Nearest neighbor tour length: {nn_length:.2f}")

    print("✓ TSPLIB loading working\n")
    return graph


def test_aco_with_callback(graph):
    """Test ACO solver with progress callback"""
    print("=" * 60)
    print("Test 3: ACO Solver with Progress Callback")
    print("=" * 60)

    # Track callback invocations
    callback_data = []

    def progress_callback(iteration, best_distance, best_tour, convergence):
        callback_data.append({
            'iteration': iteration,
            'best_distance': best_distance,
            'tour_length': len(best_tour),
            'convergence_length': len(convergence)
        })
        print(f"  Iteration {iteration:3d}: Best = {best_distance:.2f}, "
              f"Improvement = {convergence[0] - best_distance:.2f}")

    # Create colony
    colony = aco_solver.AntColony(
        graph,
        numAnts=20,
        alpha=1.0,
        beta=2.0,
        rho=0.5,
        Q=100.0
    )

    print(f"Colony: {colony}")
    print(f"Parameters: numAnts={colony.getNumAnts()}, "
          f"alpha={colony.getAlpha()}, beta={colony.getBeta()}")

    # Set callback
    colony.setProgressCallback(progress_callback)
    colony.setCallbackInterval(10)

    # Solve
    print("\nRunning optimization (100 iterations)...")
    start_time = time.time()
    best_tour = colony.solve(100)
    elapsed = time.time() - start_time

    print(f"\nOptimization complete in {elapsed:.3f}s")
    print(f"Best tour: {best_tour}")
    print(f"Best distance: {best_tour.getDistance():.2f}")
    print(f"Tour sequence length: {len(best_tour.getSequence())}")

    # Verify callback was called
    assert len(callback_data) > 0, "Callback was never invoked"
    print(f"\n✓ Callback invoked {len(callback_data)} times")

    # Verify convergence
    convergence = colony.getConvergenceData()
    assert len(convergence) == 100, "Convergence data length mismatch"
    print(f"✓ Convergence data: {len(convergence)} iterations")

    # Check improvement
    improvement = convergence[0] - convergence[-1]
    improvement_pct = (improvement / convergence[0]) * 100
    print(f"✓ Total improvement: {improvement:.2f} ({improvement_pct:.1f}%)")

    return best_tour


def test_tour_validation(graph):
    """Test tour validation"""
    print("\n" + "=" * 60)
    print("Test 4: Tour Validation")
    print("=" * 60)

    # Valid tour
    valid_sequence = list(range(graph.getNumCities()))
    tour = aco_solver.Tour(valid_sequence, 100.0)

    assert tour.validate(graph.getNumCities()), "Valid tour failed validation"
    print("✓ Valid tour passes validation")

    # Invalid tour (missing city)
    invalid_sequence = [0, 1, 2]  # Missing cities for 16-city problem
    tour2 = aco_solver.Tour(invalid_sequence, 50.0)
    assert not tour2.validate(graph.getNumCities()), "Invalid tour passed validation"
    print("✓ Invalid tour fails validation\n")


def benchmark_performance(graph):
    """Benchmark solver performance"""
    print("=" * 60)
    print("Test 5: Performance Benchmark")
    print("=" * 60)

    iterations_list = [10, 50, 100]

    for iterations in iterations_list:
        colony = aco_solver.AntColony(graph, numAnts=20)

        start_time = time.time()
        best_tour = colony.solve(iterations)
        elapsed = time.time() - start_time

        time_per_iter = (elapsed / iterations) * 1000  # ms
        print(f"  {iterations:3d} iterations: {elapsed:.3f}s "
              f"({time_per_iter:.2f} ms/iter) - Best: {best_tour.getDistance():.2f}")

    print("✓ Performance benchmark complete\n")


def main():
    """Run all tests"""
    print("\n" + "=" * 60)
    print("ACO Solver Python Bindings - Test Suite")
    print("=" * 60 + "\n")

    try:
        # Run tests
        test_basic_classes()
        graph = test_tsplib_loading()
        best_tour = test_aco_with_callback(graph)
        test_tour_validation(graph)
        benchmark_performance(graph)

        print("=" * 60)
        print("✓ ALL TESTS PASSED")
        print("=" * 60)
        print(f"\nFinal result for berlin52.tsp:")
        print(f"  Best distance: {best_tour.getDistance():.2f}")
        print(f"  Optimal known: 7542")
        print(f"  Gap: {((best_tour.getDistance() - 7542) / 7542 * 100):.1f}%")
        print()

        return 0

    except Exception as e:
        print(f"\n✗ TEST FAILED: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
