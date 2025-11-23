# ACO Solver Python Bindings

Python bindings for the high-performance C++ Ant Colony Optimization TSP Solver using pybind11.

## Installation

### Prerequisites

```bash
pip install pybind11
```

### Build

```bash
cd python_bindings
python setup.py build_ext --inplace
```

This will create `aco_solver.cpython-*.so` (Linux) or `aco_solver.*.pyd` (Windows).

## Usage

### Basic Example

```python
import aco_solver

# Load TSPLIB file
loader = aco_solver.TSPLoader("data/berlin52.tsp")
graph = loader.loadGraph()

# Create colony
colony = aco_solver.AntColony(
    graph,
    numAnts=20,
    alpha=1.0,
    beta=2.0,
    rho=0.5,
    Q=100.0
)

# Solve
best_tour = colony.solve(100)
print(f"Best distance: {best_tour.getDistance():.2f}")
```

### With Local Search (Recommended for Best Quality)

```python
colony = aco_solver.AntColony(graph, numAnts=30, alpha=1.0, beta=2.0, rho=0.5, Q=100.0)

# Enable local search for better solution quality
colony.setUseLocalSearch(True)       # Enable 2-opt/3-opt
colony.setUse3Opt(True)               # Use both 2-opt and 3-opt (default)
colony.setLocalSearchMode("best")     # Apply to best tour only (default)

# Solve
best_tour = colony.solve(100)
print(f"Best distance: {best_tour.getDistance():.2f}")
# Expected: ~7544-7607 for berlin52 (0.03-0.86% above optimal 7542)
```

### With Progress Callback

```python
# Define callback function
def progress_callback(iteration, best_distance, best_tour, convergence):
    print(f"Iteration {iteration}: Best = {best_distance:.2f}")

# Set callback
colony.setProgressCallback(progress_callback)
colony.setCallbackInterval(10)  # Call every 10 iterations

# Solve
best_tour = colony.solve(100)
```

### With Multi-Threading Control

```python
colony = aco_solver.AntColony(graph, numAnts=30)

# Enable OpenMP multi-threading (default)
colony.setUseParallel(True)
colony.setNumThreads(0)   # 0=auto-detect cores

# Or force single-threaded
colony.setUseParallel(False)
colony.setNumThreads(1)

# Or use specific thread count
colony.setUseParallel(True)
colony.setNumThreads(8)   # Use 8 threads

best_tour = colony.solve(100)
```

### Creating Custom Problems

```python
# Create cities manually
cities = [
    aco_solver.City(0, 0.0, 0.0),
    aco_solver.City(1, 10.0, 0.0),
    aco_solver.City(2, 10.0, 10.0),
    aco_solver.City(3, 0.0, 10.0)
]

# Create graph
graph = aco_solver.Graph(cities)

# Solve
colony = aco_solver.AntColony(graph, numAnts=10)
best_tour = colony.solve(50)
```

## Available Classes

### City

Represents a city with coordinates.

```python
city = aco_solver.City(id=0, x=10.0, y=20.0)
distance = city.distanceTo(other_city)
```

### Graph

TSP problem instance with precomputed distance matrix.

```python
graph = aco_solver.Graph(cities)
num_cities = graph.getNumCities()
distance = graph.getDistance(0, 1)  # O(1) lookup
```

### Tour

Solution representation.

```python
tour = best_tour
distance = tour.getDistance()
sequence = tour.getSequence()  # [0, 3, 1, 4, 2, ...]
is_valid = tour.validate(graph.getNumCities())
```

### TSPLoader

Load TSP files (auto-detects format).

```python
loader = aco_solver.TSPLoader("data/berlin52.tsp")
graph = loader.loadGraph()

# Or use specific format
graph = aco_solver.TSPLoader.loadFromTSPLIB("problem.tsp")
```

### AntColony

Main optimization algorithm.

```python
colony = aco_solver.AntColony(
    graph,
    numAnts=20,         # Number of ants
    alpha=1.0,          # Pheromone importance
    beta=2.0,           # Heuristic importance
    rho=0.5,            # Evaporation rate
    Q=100.0             # Deposit factor
)

best_tour = colony.solve(maxIterations=100)
convergence = colony.getConvergenceData()  # List of best distances per iteration
```

## Testing

Run the test suite:

```bash
cd python_bindings
python test_bindings.py
```

Expected output:
```
============================================================
ACO Solver Python Bindings - Test Suite
============================================================

✓ Test 1: Basic Class Instantiation
✓ Test 2: TSPLIB File Loading
✓ Test 3: ACO Solver with Progress Callback
✓ Test 4: Tour Validation
✓ Test 5: Performance Benchmark

============================================================
✓ ALL TESTS PASSED
============================================================
```

## Performance

The Python bindings release the GIL during C++ computation, allowing:
- Concurrent execution with other Python threads
- Near-native C++ performance
- Real-time progress callbacks via Python functions

**Benchmark (berlin52.tsp, 100 iterations):**
- ~0.65 ms/iteration
- ~65 ms total for 100 iterations
- 10-15% overhead compared to pure C++ CLI

## API Reference

All C++ classes are exposed with their full interface. Use Python's `help()` for documentation:

```python
import aco_solver
help(aco_solver.AntColony)
```

## Notes

- The module automatically searches for TSPLIB files in `data/`, `../data/`, and `../../data/`
- Distance matrix is precomputed once for O(1) lookups
- Progress callbacks are invoked from C++ code (with GIL reacquired)
- All STL containers (vectors) are automatically converted to Python lists

## Next Steps

This module is designed to be integrated with:
- Flask API for web-based visualization (see `../backend/`)
- Next.js frontend for interactive TSP solving
- Real-time WebSocket progress updates
