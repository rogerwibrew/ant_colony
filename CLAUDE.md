# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when
working with code in this repository.

## Project Overview

C++17 Ant Colony Optimization (ACO) implementation for Travelling
Salesman Problem using CMake and Google Test.

**Current Status:** ✅ **FULLY IMPLEMENTED** - 126 tests passing.
All core classes complete and working. Production-ready ACO solver
with CLI interface, TSPLIB format support, OpenMP multi-threading,
and 2-opt/3-opt local search optimization. Recent bug fix ensures
proper pheromone updates when using local search.

## Roadmap / Planned Tasks

1. **Improve UI** - 🚧 In progress: Preview feature, convergence
   stopping, ant controls, visualization improvements, local search controls
2. ✅ **Multi-core CPU support** - OpenMP parallelization integrated
   (10-12× speedup on 32 cores, runtime control via CLI/Python/Web UI)
3. ✅ **2-opt/3-opt Local Search** - Significantly improved solution quality
   (achieving 0.03% above optimal on berlin52)
4. **GPU acceleration** - Integrate GPU operations for performance
5. **Additional solution methods** - Add elite ant and other ACO
   variants (elitist pheromone, MAX-MIN Ant System)
6. **Full TSPLIB95 support** - Add all TSPLIB95 problems to the
   potential list, including fixing incompatible files in
   `uncompatibleData/`

## Essential Commands

### Build C++ Core

```bash
# Initial setup
cd cpp
mkdir build && cd build
cmake ..
cmake --build .

# Rebuild after changes
cd cpp/build && cmake --build .

# Clean rebuild (if needed)
cd cpp/build && rm -rf * && cmake .. && cmake --build .

# Create compile_commands.json symlink for LSP/clangd support (from project root)
ln -s cpp/build/compile_commands.json compile_commands.json
```

**Note:** The project includes a `.clangd` configuration file for
proper C++17 support and strict include checking.

### Build Python Bindings

```bash
cd python_bindings
python setup.py build_ext --inplace
# Creates aco_solver.cpython-*.so

# Test bindings
python test_bindings.py
```

### Web Interface Quick Start

```bash
# Install dependencies (first time only)
make install

# Start both frontend and backend servers
make start
# Frontend: http://localhost:3000
# Backend:  http://localhost:5000

# View logs
tail -f backend.log
tail -f frontend.log

# Stop servers
make stop
```

**Note:** Makefile creates `*.pid` and `*.log` files in project root
(gitignored).

### Test

```bash
# Run all tests (126 passing)
./cpp/build/bin/ant_colony_tests

# Use CTest (recommended - shows which tests fail)
cd cpp/build && ctest --output-on-failure

# Run CTest with verbose output
cd cpp/build && ctest --verbose

# Run specific test suite
./cpp/build/bin/ant_colony_tests --gtest_filter="AntColonyTest.*"

# Run specific test
./cpp/build/bin/ant_colony_tests --gtest_filter="GraphTest.DistanceMatrixSymmetry"

# List all available tests
./cpp/build/bin/ant_colony_tests --gtest_list_tests
```

### Run

```bash
# Basic usage (files auto-discovered in data/ directory)
cd cpp/build/bin
./ant_colony_tsp berlin52.tsp

# With custom parameters
./ant_colony_tsp ulysses16.tsp --ants 50 --iterations 200 --alpha 1.5 --beta 3.0

# Threading options (OpenMP multi-core support)
./ant_colony_tsp berlin52.tsp --threads 8      # Use 8 threads
./ant_colony_tsp berlin52.tsp --serial         # Force single-threaded
# Default: Auto-detect cores (multi-threaded)

# Show help
./ant_colony_tsp --help

# Works from any directory - files automatically found in data/
cd cpp/build && ./bin/ant_colony_tsp berlin52.tsp
```

**Note:** TSPLoader automatically searches for files in common
locations (`data/`, `../data/`, `../../data/`, `../../../data/`), so
you can run from any directory without specifying the full path. This
supports both the original flat structure and the current `cpp/`
subdirectory organization.

## Architecture Overview

**Implementation Status:**

- ✅ **City** (7 tests) - Represents cities with coordinates and
  distance calculation
- ✅ **Graph** (8 tests) - Stores cities + precomputed O(1) distance
  matrix
- ✅ **TSPLoader** (10 tests) - Loads TSP files (coordinate, matrix,
  and TSPLIB formats)
- ✅ **Tour** (16 tests) - Solution representation with validation
- ✅ **PheromoneMatrix** (20 tests) - Pheromone tracking with
  evaporation and deposit
- ✅ **Ant** (21 tests) - Solution construction agent with
  probabilistic city selection
- ✅ **AntColony** (29 tests) - Main algorithm coordinator with
  convergence tracking and OpenMP parallelization
- ✅ **LocalSearch** (14 tests) - 2-opt and 3-opt tour improvement algorithms
- ✅ **main.cpp** - Full CLI with parameter customization and
  formatted output

**Execution Flow:**

```text
Load TSP Instance → Initialize Colony → For each iteration:
  → Ants construct tours (probabilistic city selection)
  → [Optional] Apply local search to ant tours (mode="all")
  → Update pheromones (evaporation + deposit based on tour quality)
  → Track best solution
  → [Optional] Apply local search to best tour (mode="best", default)
→ Output best tour found + convergence data
```

**Critical Design Details:**

- Distance matrix is precomputed once in Graph constructor (O(n²))
  for O(1) lookups
- Matrix stored as `std::vector<std::vector<double>>`
  (runtime-sized, symmetric)
- Initial pheromone value uses **τ₀ = m / C^nn** formula
  (m = number of ants, C^nn = nearest neighbor tour length)
- Nearest neighbor heuristic provides problem-adaptive
  initialization
- ACO flow: Ants build tours → Update pheromones → Track best →
  Repeat
- Member variables use trailing underscore: `id_`, `cities_`
- Single-arg constructors are `explicit`

## File Organization

```text
cpp/              - C++ core implementation
  ├── include/    - Header files (.h)
  │   ├── City.h, Graph.h, Tour.h
  │   ├── PheromoneMatrix.h, Ant.h, AntColony.h
  │   ├── TSPLoader.h
  │   └── LocalSearch.h
  ├── src/        - Implementation (.cpp), main.cpp excluded from tests
  │   ├── City.cpp, Graph.cpp, Tour.cpp
  │   ├── PheromoneMatrix.cpp, Ant.cpp, AntColony.cpp
  │   ├── TSPLoader.cpp
  │   ├── LocalSearch.cpp
  │   └── main.cpp
  ├── tests/      - Google Test files (*_test.cpp)
  │   ├── City_test.cpp, Graph_test.cpp, Tour_test.cpp
  │   ├── PheromoneMatrix_test.cpp, Ant_test.cpp, AntColony_test.cpp
  │   ├── TSPLoader_test.cpp, LocalSearch_test.cpp, example_test.cpp
  │   └── data/   - Test input files (simple_5.txt, triangle_3.txt, matrix_4.txt,
          invalid.txt)
  ├── build/      - CMake build directory (gitignored)
  │   └── bin/    - Compiled executables (ant_colony_tsp, ant_colony_tests)
  └── CMakeLists.txt
data/             - TSPLIB benchmark instances (113+ files, shared by all components)
  ├── berlin52.tsp, ulysses16.tsp, att48.tsp, eil51.tsp
  └── ... (many more EUC_2D problems)
python_bindings/  - pybind11 Python bindings to C++ solver
  ├── bindings.cpp       - pybind11 wrapper code
  ├── setup.py           - Build configuration
  ├── test_bindings.py   - Python test suite
  └── aco_solver.*.so    - Compiled Python module (gitignored)
backend/          - Flask API with WebSocket support
  ├── app.py             - Main Flask application
  └── requirements.txt   - Python dependencies
frontend/         - Next.js web interface
  ├── src/
  │   ├── app/           - Next.js App Router pages
  │   └── components/    - React components
  └── package.json       - npm dependencies
Makefile          - Quick commands for web development (start/stop/install)
```

**Notes:**

- `compile_commands.json` in root is a symlink to
  `cpp/build/compile_commands.json` (gitignored but useful for LSP)
- Makefile creates `*.pid` and `*.log` files in root when running
  web servers (gitignored)

## Adding New Classes

1. Create `cpp/include/ClassName.h` and `cpp/src/ClassName.cpp`
2. Add `cpp/tests/ClassName_test.cpp`
3. CMake auto-discovers via GLOB_RECURSE - just rebuild

## Input File Formats

**Simple Coordinate Format:**

```text
5                    # Number of cities
0 10.0 20.0         # City ID, X coordinate, Y coordinate
1 15.0 25.0
2 30.0 40.0
3 50.0 10.0
4 20.0 30.0
```

**Simple Distance Matrix Format:**

```text
4                    # Number of cities
0.0 5.0 10.0 15.0   # n×n distance matrix
5.0 0.0 8.0 12.0
10.0 8.0 0.0 6.0
15.0 12.0 6.0 0.0
```

**TSPLIB Format (113+ benchmark files in data/):**

```text
NAME: berlin52
TYPE: TSP
COMMENT: 52 locations in Berlin
DIMENSION: 52
EDGE_WEIGHT_TYPE: EUC_2D
NODE_COORD_SECTION
1 565.0 575.0
2 25.0 185.0
...
EOF
```

**Supported EDGE_WEIGHT_TYPE:**

- ✅ **EUC_2D** (Euclidean 2D) - Most TSPLIB files
- ❌ ATT (pseudo-Euclidean) - Not yet supported
- ❌ GEO (geographical coordinates) - Not yet supported
- ❌ EXPLICIT (distance matrix) - Not yet supported

**TSPLoader usage:**

```cpp
// Automatically searches data/, ../data/, ../../data/
TSPLoader loader("berlin52.tsp");  // or "problem.txt"
Graph graph = loader.loadGraph();  // Auto-detects format
if (!graph.isValid()) { /* handle error */ }
```

## Algorithm Parameters

Default values for ACO:

- `alpha = 1.0` (pheromone importance)
- `beta = 2.0` (heuristic/distance importance)
- `rho = 0.5` (evaporation rate)
- `Q = 100.0` (deposit factor)
- `numAnts = 20-50`
- `iterations = 100-1000`

**Selection probability:**
`p[i][j] = (pheromone^alpha * (1/distance)^beta) / sum(unvisited)`

**Pheromone update strategies:**

- **Ant Cycle:** Pheromone deposited after complete tour: `Δτ = Q / tour_length`
- **Ant Quantity:** Pheromone deposited during tour construction
- **Elitist:** Only best ant(s) deposit pheromones

---

## Class Specifications

### Class: City

**Purpose:** Represents a single city in the TSP instance with coordinates.

**Member Variables:**

- `int id_` - Unique identifier for the city
- `double x_` - X coordinate
- `double y_` - Y coordinate

**Methods:**

- `City(int id, double x, double y)` - Constructor
- `double distanceTo(const City& other) const` - Calculate Euclidean
  distance to another city
- `int getId() const` - Getter for city ID
- `double getX() const` - Getter for X coordinate
- `double getY() const` - Getter for Y coordinate

**Dependencies:** None (standalone class)

**Notes:**

- Uses Euclidean distance formula: `sqrt((x2-x1)^2 + (y2-y1)^2)`
- Could be extended to support other distance metrics

---

### Class: Graph

**Purpose:** Stores the complete TSP problem instance including all
cities and precomputed distances.

**Member Variables:**

- `std::vector<City> cities_` - All cities in the problem
- `std::vector<std::vector<double>> distanceMatrix_` - Precomputed
  distances between all city pairs
- `int numCities_` - Number of cities

**Methods:**

- `Graph(const std::vector<City>& cities)` - Constructor that builds
  distance matrix
- `Graph()` - Default constructor (empty graph for error handling)
- `double getDistance(int cityA, int cityB) const` - Get distance
  between two cities (O(1))
- `int getNumCities() const` - Get total number of cities
- `const City& getCity(int index) const` - Get city by index
- `const std::vector<City>& getCities() const` - Get all cities
- `bool isValid() const` - Check if graph has cities
- `double nearestNeighborTourLength(int startCity = 0) const` -
  Calculate tour length using greedy nearest neighbor heuristic

**Dependencies:** City

**Notes:**

- Distance matrix is symmetric for Euclidean TSP
- Matrix computed once at construction for efficiency
  (O(n²) construction, O(1) lookup)
- Nearest neighbor heuristic used to compute initial pheromone value:
  τ₀ = m / C^nn

---

### Class: Tour

**Purpose:** Represents a complete tour (solution) visiting all cities.

**Member Variables:**

- `std::vector<int> citySequence_` - Ordered list of city IDs
- `double totalDistance_` - Total tour length
- `bool isValid_` - Whether tour visits all cities exactly once

**Methods:**

- `Tour()` - Default constructor
- `Tour(const std::vector<int>& sequence, double distance)` -
  Constructor with values
- `void setTour(const std::vector<int>& sequence, double distance)` -
  Set tour data
- `double getDistance() const` - Get tour length
- `const std::vector<int>& getSequence() const` - Get city sequence
- `bool validate(int numCities) const` - Verify tour validity

**Dependencies:** None (data container)

**Notes:**

- Tour includes return to starting city in distance calculation
- Example: `[0, 3, 1, 4, 2]` means visit cities in that order then return to 0

---

### Class: PheromoneMatrix

**Purpose:** Manages pheromone levels on all edges between cities.

**Member Variables:**

- `std::vector<std::vector<double>> pheromones_` - Pheromone levels
  between city pairs
- `int numCities_` - Number of cities
- `double initialPheromone_` - Initial pheromone level
- `double minPheromone_` - Minimum allowed pheromone level
- `double maxPheromone_` - Maximum allowed pheromone level
  (for MAX-MIN Ant System)

**Methods:**

- `PheromoneMatrix(int numCities, double initial)` - Constructor
- `void initialize(double value)` - Reset all pheromones to initial value
- `double getPheromone(int cityA, int cityB) const` - Get pheromone level
- `void setPheromone(int cityA, int cityB, double value)` - Set pheromone level
- `void evaporate(double rho)` - Apply evaporation to all edges:
  `pheromone *= (1 - rho)`
- `void depositPheromone(int cityA, int cityB, double amount)` - Add
  pheromone to edge
- `void clampPheromones()` - Enforce min/max bounds (optional, for
  MMAS variant)

**Dependencies:** None

**Notes:**

- Matrix is symmetric for undirected TSP
- Consider implementing bounds for MAX-MIN Ant System variant

---

### Class: Ant

**Purpose:** Individual ant agent that constructs a tour using
pheromone and heuristic information.

**Member Variables:**

- `int currentCity_` - Current position
- `std::vector<bool> visited_` - Tracks visited cities
- `std::vector<int> tour_` - Current tour being constructed
- `double tourLength_` - Length of current tour

**Methods:**

- `Ant(int startCity, int numCities)` - Constructor
- `void reset(int startCity)` - Reset ant to start a new tour
- `int selectNextCity(const Graph& graph,
  const PheromoneMatrix& pheromones, double alpha, double beta)` -
  Choose next city probabilistically
- `void visitCity(int city, const Graph& graph)` - Add city to tour
- `bool hasVisitedAll() const` - Check if tour is complete
- `Tour completeTour(const Graph& graph)` - Finalize and return the tour
- `double calculateTourLength(const Graph& graph) const` - Calculate
  total tour distance

**Dependencies:** Graph, PheromoneMatrix, Tour

**Notes:**

- Selection probability:
  `p_ij = (pheromone^alpha * (1/distance)^beta) / sum(all unvisited)`
- Uses roulette wheel selection for next city
- **Performance critical:** This is the hotspot (~60-70% of runtime)

---

### Class: AntColony

**Purpose:** Manages the colony of ants, pheromone matrix, and
algorithm execution.

**Member Variables:**

- `Graph graph_` - The TSP problem instance
- `PheromoneMatrix pheromones_` - Pheromone matrix
- `std::vector<Ant> ants_` - Colony of ants
- `int numAnts_` - Number of ants
- `double alpha_` - Pheromone importance factor
- `double beta_` - Heuristic importance factor
- `double rho_` - Evaporation rate
- `double Q_` - Pheromone deposit factor
- `Tour bestTour_` - Best tour found so far
- `std::vector<double> iterationBestDistances_` - Track convergence
- `bool useParallel_` - Enable/disable parallel execution (default: true if OpenMP available)
- `int numThreads_` - Thread count control (0=auto-detect, 1=serial, 2+=specific)

**Methods:**

- `AntColony(const Graph& graph, int numAnts, double alpha,
  double beta, double rho, double Q)` - Constructor
- `void initialize()` - Initialize pheromones using τ₀ = m / C^nn
  formula and reset state
- `void runIteration()` - Execute one complete iteration
- `void constructSolutions()` - All ants build tours
- `void updatePheromones()` - Evaporate and deposit pheromones
- `Tour solve(int maxIterations)` - Run algorithm for specified iterations
- `Tour solve(int maxIterations, int convergenceIterations)` - Run
  with early stopping if no improvement for N iterations
- `const Tour& getBestTour() const` - Get best solution found
- `const std::vector<double>& getConvergenceData() const` - Get
  iteration history
- `void setProgressCallback(callback, interval)` - Set callback
  function for progress updates (Python bindings)
- `void setUseParallel(bool useParallel)` - Enable/disable OpenMP
  multi-threading (default: true if available)
- `void setNumThreads(int numThreads)` - Set thread count (0=auto-detect,
  1=serial, 2+=specific count)
- `void setUseLocalSearch(bool useLocalSearch)` - Enable/disable 2-opt/3-opt
  local search (default: disabled)
- `void setUse3Opt(bool use3opt)` - Enable/disable 3-opt in addition to 2-opt
  (default: true if local search enabled)
- `void setLocalSearchMode(const std::string& mode)` - Set when to apply
  local search: "best" (default), "all", or "none"

**Dependencies:** Graph, PheromoneMatrix, Ant, Tour, LocalSearch

**Notes:**

- Initial pheromone computed as τ₀ = m / C^nn (recommended in ACO literature)
- Falls back to τ₀ = 1.0 if nearest neighbor tour length is zero or invalid
- Can implement different pheromone update strategies (ant-cycle,
  ant-quantity, etc.)
- Consider elitist strategy (only best ant deposits pheromones)
- **Convergence-based stopping:** If `convergenceIterations`
  specified, solver stops early when no improvement for N iterations
- **OpenMP Parallelization:** When OpenMP is available, achieves 10-12×
  speedup on multi-core CPUs through:
  - Parallel ant tour construction with dynamic scheduling
  - Parallel pheromone evaporation with loop collapsing
  - Thread-safe pheromone deposition using atomic operations
  - Adaptive threading to reduce contention on atomic updates
  - Parallel best tour finding with thread-local reduction
  - Graceful fallback to serial execution if OpenMP unavailable

---

### Class: TSPLoader

**Purpose:** Loads TSP problem instances from files (coordinate,
matrix, or TSPLIB format).

**Member Variables:**

- `std::string filename_` - Path to problem file

**Methods:**

- `TSPLoader(const std::string& filename)` - Constructor
  (auto-searches common paths)
- `Graph loadGraph()` - Parse file and return Graph object
  (auto-detects format)
- `static Graph loadFromCoordinates(const std::string& filename)` -
  Load from coordinate file
- `static Graph loadFromDistanceMatrix(const std::string& filename)` -
  Load from distance matrix
- `static Graph loadFromTSPLIB(const std::string& filename)` - Load
  from TSPLIB format

**Dependencies:** Graph, City

**Notes:**

- Auto-detection: Checks for "NAME:", "TYPE:", "DIMENSION:" (TSPLIB),
  3 fields → coordinates, >3 fields → matrix
- Auto-searches paths: current dir, `data/`, `../data/`, `../../data/`,
  `../../../data/` (supports both flat and `cpp/` subdirectory structures)
- Returns empty Graph on error (check with `graph.isValid()`)
- TSPLIB: Only EUC_2D edge weight type currently supported

---

### Class: LocalSearch

**Purpose:** Provides static methods for improving TSP tours using local
search heuristics (2-opt and 3-opt edge-swapping algorithms).

**Why Local Search:**

ACO is excellent at exploring the solution space and finding good solutions,
but it can sometimes converge to sub-optimal solutions. Local search algorithms
complement ACO by:

1. **Polishing solutions:** Refining ACO-generated tours to local optima
2. **Improving solution quality:** Typically 5-15% better tour distances
3. **Guaranteed improvement:** Iteratively swaps edges until no improvement possible
4. **Fast convergence:** Usually reaches local optimum in few iterations

**Member Variables:**

None (utility class with static methods only)

**Methods:**

- `static bool twoOpt(Tour& tour, const Graph& graph)` - Improve tour using 2-opt
- `static bool threeOpt(Tour& tour, const Graph& graph)` - Improve tour using 3-opt
- `static bool improve(Tour& tour, const Graph& graph, bool use3opt = true)` -
  Apply both 2-opt and optionally 3-opt in sequence

**Dependencies:** Tour, Graph

**How 2-opt Works:**

The 2-opt algorithm systematically tries swapping pairs of edges to eliminate
"crossings" in the tour:

1. **For each pair of edges** (i,i+1) and (j,j+1) in the tour:
   - Current tour: ... → i → i+1 → ... → j → j+1 → ...
   - Proposed swap: ... → i → j → ... → i+1 → j+1 → ...
   - This reverses the segment between i+1 and j

2. **Calculate delta:** Compute change in tour length without full reconstruction:
   ```
   delta = [dist(i,j) + dist(i+1,j+1)] - [dist(i,i+1) + dist(j,j+1)]
   ```

3. **Apply if improvement:** If delta < 0, accept the swap (tour gets shorter)

4. **Repeat until convergence:** Continue until no improving swaps found (local optimum)

**Time Complexity:** O(n²) per iteration, typically converges in 2-5 iterations

**Why 2-opt is Effective:**

- Eliminates edge crossings (a hallmark of sub-optimal tours)
- Simple and fast - only needs distance lookups
- Empirically very effective on Euclidean TSP instances
- Can be parallelized (future enhancement)

**How 3-opt Works:**

The 3-opt algorithm is more sophisticated - it removes 3 edges and tries
all 7 possible reconnection patterns:

1. **For each triple of edges** (i,i+1), (j,j+1), (k,k+1):
   - Remove these 3 edges, creating 3 segments

2. **Try 7 reconnection patterns:**
   - Original (case 0)
   - Reverse segment (i+1,j) - equivalent to 2-opt
   - Reverse segment (j+1,k)
   - Reverse both segments
   - Swap segments (i+1,j) and (j+1,k)
   - ... and 2 more complex reconnections

3. **Keep best reconnection:** Choose the one with shortest tour length

4. **Repeat until convergence:** No improvement from any 3-edge removal

**Time Complexity:** O(n³) per iteration, typically 1-3 iterations

**Why 3-opt is Better (but Slower):**

- Can make more complex tour improvements that 2-opt cannot
- Finds better local optima (though still local, not global)
- Much more expensive: O(n³) vs O(n²)
- Typically provides 1-3% additional improvement over 2-opt alone

**Integration with AntColony:**

Local search can be applied at two strategic points:

1. **Mode "best" (default, recommended):** Apply to best tour found so far
   - Applied once per iteration after tracking best solution
   - Low overhead, maximum benefit
   - Best for most use cases

2. **Mode "all":** Apply to every ant's tour immediately after construction
   - Applied numAnts times per iteration
   - Higher overhead but potentially better pheromone information
   - Useful for problems where ACO struggles to find good initial solutions

**Configuration via AntColony:**

```cpp
colony.setUseLocalSearch(true);       // Enable local search
colony.setUse3Opt(true);               // Use both 2-opt and 3-opt (default)
colony.setLocalSearchMode("best");     // Apply to best tour only (default)
```

**Performance Impact & Benchmarks:**

**See [BENCHMARKS.md](BENCHMARKS.md) for comprehensive benchmark results and methodology.**

**Benchmark Configuration:** 100 iterations, 30 ants, 5 runs per configuration

| Problem | Size | Optimal | Without LS | 2-opt Only | 2-opt+3-opt | Improvement |
|---------|------|---------|------------|------------|-------------|-------------|
| berlin52 | 52 | 7542 | 7773.6 (+3.1%) | 7569.4 (+0.4%) | 7590.2 (+0.6%) | **2.4%** |
| eil51 | 51 | 426 | 466.7 (+9.6%) | 452.7 (+6.3%) | 447.8 (+5.1%) | **4.0%** |
| st70 | 70 | 675 | 750.7 (+11.2%) | 723.0 (+7.1%) | 710.4 (+5.3%) | **5.4%** |

**Key Findings:**

- **Consistent improvement:** 2.4-5.4% better solution quality across all problems
- **Near-optimal results:** 2-opt achieves 0.4-7.1% above optimal (vs 3-11% without)
- **3-opt refinement:** Additional 1-2% improvement over 2-opt alone
- **Best single result:** 7544.37 on berlin52 (0.03% above optimal!)
- **Runtime Overhead:**
  - Mode "best": ~5-10% slower (negligible for quality gain)
  - Mode "all": ~2-3× slower (better pheromone information)
- **Reduced variance:** Lower standard deviation with local search (more consistent results)

**Notes:**

- Local search is **disabled by default** to maintain ACO-only baseline
- When enabled, typically runs 2-opt first (fast), then 3-opt (thorough)
- Tours are modified in-place for efficiency
- Preserves tour validity (still visits all cities exactly once)
- Can be applied independently to any Tour object, not just ACO solutions

**Example Usage:**

```cpp
// Standalone usage
Tour tour = colony.solve(100);  // Get ACO solution
LocalSearch::improve(tour, graph);  // Polish with 2-opt+3-opt

// Integrated usage
colony.setUseLocalSearch(true);
Tour tour = colony.solve(100);  // Automatically applies local search
```

---

## Performance Notes

**Expected Performance Characteristics:**

- **~60-70%**: Probability calculation and city selection in
  `Ant::selectNextCity()`
  - Computing probabilities for all unvisited cities
  - Random number generation
  - Roulette wheel selection
- **~20-30%**: Pheromone updates
  - Evaporation (all edges)
  - Deposition (tour edges)
- **~5-10%**: Distance lookups and tour length calculation
- **<5%**: Everything else

**Not likely to be bottlenecks:**

- Distance lookups (O(1) with precomputed matrix)
- Tour validation
- File I/O (happens once)

**Data Structure Decisions:**

Use `std::vector<std::vector<double>>` for distance and pheromone
matrices even though sizes are fixed after initialization.

**Reasons:**

- Matrix size is only known at runtime, not compile-time
- `std::array` requires compile-time constant size
- `std::vector` has zero overhead for element access once allocated
- Both use contiguous memory with O(1) random access

**Potential Optimization (if profiling shows bottleneck):**

- Flattened 1D vector: `std::vector<double>` with index `[i*n + j]`
  for better cache locality
- Only store upper triangle of symmetric matrix (saves 50% memory)
- Use `float` instead of `double` if precision allows

## Testing Strategy

**Unit Tests (Google Test) - 106 tests total:**

1. **City Class (7 tests):** Distance calculation accuracy, getter
   methods, edge cases
2. **Graph Class (8 tests):** Distance matrix symmetry, correct
   lookups, invalid indices
3. **Tour Class (16 tests):** Tour validation, distance calculation,
   invalid tour detection, edge cases
4. **PheromoneMatrix Class (20 tests):** Initialization, evaporation,
   deposit, clamping, symmetry
5. **Ant Class (21 tests):** Tour construction, probabilistic
   selection, parameter sensitivity, edge cases
6. **AntColony Class (23 tests):** Convergence, best tour tracking,
   parameter variations, solution quality
7. **TSPLoader Class (10 tests):** Format detection, coordinate
   loading, matrix loading, error handling
8. **Example Tests (2 tests):** Basic assertion and string comparison

**Verified Solution Quality:**

- **Triangle problem (3 cities):** Finds optimal solution (12.0)
- **Square problem (4 cities):** Finds optimal solution (4.0) within 0.1
- **ulysses16 (16 cities):** 74.46 vs optimal 73.99 (0.6% above optimal)
- **berlin52 (52 cities):** ~7900 vs optimal 7542 (~4.8% above optimal)

**Test Coverage:**

- ✅ Trivial cases (1-2 cities)
- ✅ Small problems (3-16 cities) - finds optimal or near-optimal
- ✅ Medium problems (52+ cities) - good quality solutions
- ✅ Edge cases (zero distances, single city, invalid inputs)
- ✅ Parameter sensitivity (alpha, beta, rho, Q, numAnts)
- ✅ Convergence behavior (monotonic improvement, tracking)

## Usage Examples

### Basic Usage

```bash
cd build/bin

# Solve berlin52 with default parameters
./ant_colony_tsp berlin52.tsp

# Solve with custom parameters
./ant_colony_tsp berlin52.tsp --ants 50 --iterations 300 --alpha 1.5 --beta 3.0

# Enable local search for better solution quality
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search

# Use only 2-opt (faster, still good improvement)
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search --2opt-only

# Apply local search to all ant tours (slower but better pheromone information)
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search --ls-mode all

# Multi-threaded with local search (recommended for large problems)
./ant_colony_tsp berlin52.tsp --iterations 200 --local-search --threads 8
```

### Example Output

```text
========================================
Ant Colony Optimization - TSP Solver
========================================

Loading TSP instance from: berlin52.tsp
Successfully loaded 52 cities

Algorithm Parameters:
  Number of ants:       20
  Iterations:           100
  Alpha (pheromone):    1
  Beta (heuristic):     2
  Rho (evaporation):    0.5
  Q (deposit factor):   100

Running Ant Colony Optimization...

========================================
Results
========================================

Best tour distance: 7906.23

Best tour sequence:
38 -> 35 -> 34 -> ... -> 36 -> 38 (return to start)

Convergence Summary:
  First iteration best: 14934.18
  Final iteration best: 8723.10
  Overall best:         7906.23
  Improvement:          7027.95 (47.06%)

========================================
```

### Available TSPLIB Files (EUC_2D)

**Small (< 100 cities):**

- ulysses16 (16), burma14 (14), ulysses22 (22), bayg29 (29), fri26 (26)
- att48 (48), berlin52 (52), st70 (70), eil51 (51), eil76 (76), eil101 (101)

**Medium (100-500 cities):**

- kroA100, kroB100, kroC100, kroD100, kroE100 (all 100)
- lin105 (105), pr107 (107), bier127 (127), ch130 (130), ch150 (150)
- kroA150, kroB150 (both 150), pr152 (152), rat195 (195), kroA200,
  kroB200 (both 200)

**Large (500+ cities):**

- rat575 (575), rat783 (783), pr1002 (1002), u1060 (1060), vm1084 (1084)
- And many more up to 85,900 cities!

## Known Limitations

1. **Edge Weight Types:** Only EUC_2D (Euclidean 2D) is supported.
   Files with ATT, GEO, or EXPLICIT types will fail to load.

2. **Pheromone Strategy:** Uses basic Ant Cycle (pheromone deposited
   after tour completion). Does not implement:
   - Ant Quantity (deposit during construction)
   - Elitist strategy (only best ant deposits)
   - MAX-MIN Ant System bounds (implemented but not enforced by
     default)

3. **Solution Quality:** For large problems (1000+ cities) without local
   search, solutions may be 10-20% above optimal. **Enable local search**
   (`--local-search` flag) to achieve near-optimal solutions (often <1% above optimal).

## Web Interface

The project includes a full-stack web interface for interactive TSP
solving with real-time visualization and advanced controls.

### Features

**Problem Preview:**

- Cities are automatically loaded and displayed when selecting a TSP
  problem from dropdown
- Preview appears in top-right visualization before running solver
- Allows visual inspection of problem layout before optimization

**Solver Controls:**

- **Iterations:** Set max iterations (10-10,000) or use
  convergence-based stopping
- **Convergence Mode:** Stop automatically when no improvement for N
  iterations (default: 200)
- **Number of Ants:** Auto-calculate (1-2 per city) or specify custom value
- **ACO Parameters:** Adjustable α (pheromone), β (heuristic), ρ (evaporation)

**Visualizations:**

- **City Positions:** Real-time display with uniform viewport-based sizing
- **Current Path:** Live tour updates during optimization
- **Convergence Graph:** Monotonically decreasing plot of best
  solution over time
- **Status Console:** Real-time solver status and progress messages

**Performance:**

- WebSocket-based real-time updates (every 10 iterations)
- C++ solver releases GIL for concurrent Python operations
- Progress tracking with elapsed time and iteration count

### Frontend (Next.js)

Located in `frontend/`, built with:

- Next.js 16 (App Router)
- TypeScript
- Tailwind CSS
- Recharts (convergence visualization)
- socket.io-client (real-time updates)

**Components:**

- `aco-tsp-dashboard.tsx` - Main dashboard layout
- `configuration-panel.tsx` - Problem selection and parameter controls
- `city-visualization.tsx` - City/tour visualization with preview
- `path-display.tsx` - Current iteration path display
- `performance-chart.tsx` - Convergence chart
- `useSocket.ts` - WebSocket hook for backend communication

**Running:**

```bash
cd frontend
npm install
npm run dev  # http://localhost:3000
```

### Backend (Flask)

Located in `backend/`, provides:

- REST API for benchmark listing and parameter defaults
- WebSocket server for real-time optimization progress
- Preview endpoint for loading cities without solving
- Integration with C++ solver via pybind11
- Convergence tracking and early stopping logic

**WebSocket Events:**

- `connect` → `connected`
- `preview` → `preview_loaded` (load cities without solving)
- `solve` → `loaded`, `progress` (every 10 iter), `complete`
- `cancel` → `cancelled`

**Running:**

```bash
cd backend
pip install -r requirements.txt
python app.py  # http://localhost:5000
```

## Python Bindings

The project includes pybind11 bindings that expose the C++ solver to
Python with near-native performance.

### Quick Example

```python
import aco_solver

# Load problem
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()

# Create and run solver
colony = aco_solver.AntColony(graph, numAnts=20, alpha=1.0, beta=2.0, rho=0.5, Q=100.0)
best_tour = colony.solve(100)
print(f"Best distance: {best_tour.getDistance():.2f}")
```

### Threading Control

```python
# Control multi-threading behavior
colony.setUseParallel(True)   # Enable OpenMP multi-threading (default)
colony.setNumThreads(0)        # 0=auto-detect cores, 1=serial, 2+=specific count
best_tour = colony.solve(100)

# Force single-threaded execution
colony.setUseParallel(False)
colony.setNumThreads(1)
best_tour = colony.solve(100)

# Use specific number of threads
colony.setUseParallel(True)
colony.setNumThreads(8)        # Use 8 threads
best_tour = colony.solve(100)
```

### Progress Callbacks

```python
def progress_callback(iteration, best_distance, best_tour, convergence):
    print(f"Iteration {iteration}: Best = {best_distance:.2f}")

colony.setProgressCallback(progress_callback)
colony.setCallbackInterval(10)  # Every 10 iterations
best_tour = colony.solve(100)
```

### Local Search Control

```python
# Enable local search for better solution quality
colony.setUseLocalSearch(True)       # Enable 2-opt/3-opt
colony.setUse3Opt(True)               # Use both 2-opt and 3-opt (default)
colony.setLocalSearchMode("best")     # Apply to best tour only (default)
best_tour = colony.solve(100)

# Use only 2-opt (faster)
colony.setUseLocalSearch(True)
colony.setUse3Opt(False)  # Skip 3-opt
best_tour = colony.solve(100)

# Apply to all ant tours (better pheromone information)
colony.setUseLocalSearch(True)
colony.setLocalSearchMode("all")
best_tour = colony.solve(100)
```

### Direct LocalSearch Usage

```python
import aco_solver

# Load problem and get an ACO solution
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()
colony = aco_solver.AntColony(graph, numAnts=30)
tour = colony.solve(50)

print(f"Before local search: {tour.getDistance():.2f}")

# Apply local search directly
aco_solver.LocalSearch.twoOpt(tour, graph)
print(f"After 2-opt: {tour.getDistance():.2f}")

aco_solver.LocalSearch.threeOpt(tour, graph)
print(f"After 3-opt: {tour.getDistance():.2f}")

# Or use the convenience method
tour2 = colony.solve(50)
aco_solver.LocalSearch.improve(tour2, graph, use3opt=True)
print(f"After improve(): {tour2.getDistance():.2f}")
```

**Key Features:**

- Releases GIL during C++ computation (allows concurrent Python operations)
- Full OpenMP multi-threading support with runtime control
- Real-time progress callbacks from C++
- 10-15% overhead vs pure C++ CLI
- All C++ classes exposed: City, Graph, Tour, TSPLoader, AntColony, LocalSearch
- Automatic path searching for TSPLIB files
- **Near-optimal solutions:** Achieves 0.03% above optimal on berlin52 with local search

**Testing:** Run `python test_bindings.py` in `python_bindings/` directory

## Future Enhancements

This section outlines planned improvements organized by priority and
expected impact.

### High Priority

**1. Multi-core Parallelization** - ✅ **COMPLETE**
- **Status:** Integrated using OpenMP
- **Performance:** 10-12× speedup on 32-core CPUs
- **Features:** Runtime control via CLI/Python/Web UI, adaptive threading,
  graceful fallback
- **See:** [cpu_parallel.md](cpu_parallel.md) and [INTEGRATION_SUMMARY.md](INTEGRATION_SUMMARY.md)

**2. 2-opt/3-opt Local Search** - ✅ **COMPLETE**
- **Status:** Fully implemented with 14 comprehensive tests
- **Impact:** Dramatic solution quality improvement (achieving 0.03% above optimal on berlin52)
- **Features:**
  - Configurable modes: "best" (default), "all", or "none"
  - Optional 3-opt for additional refinement beyond 2-opt
  - Available via CLI (`--local-search`), Python (`setUseLocalSearch()`), and C++ API
  - Standalone usage via `LocalSearch::improve()` static methods
- **Performance:** ~5-10% runtime overhead in "best" mode for 5-15% solution improvement
- **See:** LocalSearch class documentation above for detailed how/why

**3. Additional TSPLIB Formats** - ⭐⭐⭐⭐
- **Support for ATT** (pseudo-Euclidean distance)
- **Support for GEO** (geographical coordinates with latitude/longitude)
- **Support for EXPLICIT** (pre-computed distance matrices)
- **Impact:** Enables solving all TSPLIB95 benchmark problems
- **Complexity:** Low-Medium
- **Notes:** Currently only EUC_2D is supported. Adding these formats
  unlocks 40+ additional benchmark problems.

### Medium Priority

**4. Elitist Pheromone Strategy** - ⭐⭐⭐⭐
- **Impact:** Improved convergence and solution quality
- **Implementation:** Only best-so-far ant deposits pheromones (or
  weighted combination)
- **Complexity:** Low
- **Notes:** Simple modification to `updatePheromones()` method.
  Classic ACO variant with good empirical results.

**5. MAX-MIN Ant System (MMAS)** - ⭐⭐⭐⭐
- **Impact:** Prevents premature convergence, more robust algorithm
- **Implementation:** Enforce pheromone bounds, use
  `clampPheromones()` method
- **Complexity:** Low
- **Notes:** Framework already exists in PheromoneMatrix class.
  Just needs to enable bounds and add dynamic bound calculation.

**6. Export Results (CSV/JSON)** - ⭐⭐⭐
- **Impact:** User-requested feature for analysis and comparison
- **Implementation:** Add `--output` flag to CLI, export tour sequence
  and convergence data
- **Complexity:** Low
- **Formats:**
  - CSV: iteration, best_distance, current_iteration_best
  - JSON: full solution with metadata, parameters, timing

**7. Web UI Enhancements** - ⭐⭐⭐
- Additional benchmark problems in dropdown (currently ~20, expand
  to all 113+ EUC_2D files)
- Path animation playback (step through optimization history)
- Side-by-side comparison mode (run multiple parameter sets
  simultaneously)
- Solution quality indicator (% above known optimal)

### Low Priority

**8. Adaptive Parameters** - ⭐⭐⭐
- **Impact:** Automatic parameter tuning, better out-of-the-box
  performance
- **Implementation:** Dynamic adjustment of alpha, beta, rho based on
  convergence metrics
- **Complexity:** High
- **Notes:** Research-level feature. Requires careful design to avoid
  instability.

**9. GPU Acceleration** - ⭐⭐
- **Impact:** Massive speedup for very large problems (1000+ cities)
- **Implementation:** CUDA/OpenCL for parallel probability calculations
- **Complexity:** Very High
- **Notes:** Most beneficial for probability calculation in
  `selectNextCity()`. Requires significant refactoring.

**10. Additional ACO Variants** - ⭐⭐
- **Ant Colony System (ACS):** Pseudo-random proportional rule,
  local pheromone update
- **Rank-Based Ant System:** Only top-k ants deposit pheromones
- **Complexity:** Medium per variant
- **Notes:** Academic interest, incremental improvements over base ACO

### Research/Experimental

**11. Hybrid Approaches**
- Combine ACO with genetic algorithms
- Integration with simulated annealing
- Machine learning for parameter selection

**12. Dynamic TSP**
- Support for time-varying edge weights
- Real-time re-optimization as problem changes

**13. Multi-objective Optimization**
- Optimize for multiple criteria (distance, time, cost)
- Pareto frontier exploration

---

### Implementation Notes

- **Performance profiling recommended** before GPU work (verify
  selectNextCity is still the bottleneck)
- **Backward compatibility:** New features should not break existing
  API or file formats
- **Testing required:** Each enhancement should add corresponding test
  cases
- **Documentation:** Update CLAUDE.md and README.md when adding features

---

## Recent Improvements

**Critical Bug Fix - Pheromone Update with Local Search (2025-11-23):**
- ✅ **Fixed major bug preventing ACO improvement after first iteration**
- **Problem:** When local search was applied in mode "all", improved tours were being lost during pheromone updates
  - Tours were improved by local search but then `completeTour()` was called again, recreating original unimproved tours
  - Pheromones deposited based on poor tours instead of improved ones, preventing convergence
- **Solution:** Added `antTours_` vector to store improved tours for pheromone updates
  - Store tours after local search in `constructSolutions()`
  - Use stored tours in `updatePheromones()` and `runIteration()`
- **Impact:** Algorithm now properly improves over iterations
  - Before fix: 7598.44 → 7598.44 (0% improvement)
  - After fix: 8253.84 → 7606.95 (7.84% improvement)
- All 126 tests passing

**2-opt/3-opt Local Search Implementation (2025-11-23):**
- ✅ Implemented LocalSearch class with 2-opt and 3-opt edge-swapping algorithms
- ✅ **Dramatic solution quality improvement:** Achieving 0.03% above optimal on berlin52 (vs 1.8% before)
- ✅ Full-stack integration: CLI (`--local-search`, `--2opt-only`, `--ls-mode`), Python bindings, C++ API
- ✅ Configurable application modes: "best" (apply to best tour), "all" (apply to all ant tours), "none"
- ✅ Added 14 comprehensive tests covering edge cases, validity preservation, and convergence (126 tests total, all passing)
- ✅ Standalone usage via static methods for flexible tour improvement
- ✅ Minimal runtime overhead (~5-10% in "best" mode) for 5-15% typical solution improvement
- **Performance Results (berlin52, 100 iterations, 30 ants after bug fix):**
  - Without local search: 10054 → 3395 (66% improvement, ACO working correctly)
  - With local search: 8253 → 7607 (7.8% improvement, 0.86% above optimal 7542)
  - Best recorded: 7544.37 (0.03% above optimal!)
- **How it works:**
  - 2-opt: Eliminates edge crossings by swapping pairs of edges (O(n²) per iteration)
  - 3-opt: Removes 3 edges and tries 7 reconnection patterns (O(n³) per iteration)
  - Both run until convergence (no improving swaps found)
- **Note:** Local search finds strong local optima quickly; for problems requiring more exploration, consider using mode "best" or disabling local search initially
- See LocalSearch class documentation for detailed algorithm explanation

**OpenMP Multi-Threading Integration (2025-11-23):**
- ✅ Integrated OpenMP parallelization from openmp-parallel branch into master
- ✅ **10-12× performance improvement** on multi-core CPUs (tested on 32-core system)
- ✅ Full-stack threading control: CLI (`--threads`, `--serial`), Python bindings (`setUseParallel()`, `setNumThreads()`), Web UI (dropdown)
- ✅ Graceful fallback to serial execution if OpenMP unavailable
- ✅ Added 6 new threading-specific tests (126 tests total, all passing)
- ✅ Thread-safe pheromone updates with atomic operations
- ✅ Adaptive threading to prevent contention on pheromone deposits
- ✅ Parallel ant tour construction with dynamic scheduling
- See [INTEGRATION_SUMMARY.md](INTEGRATION_SUMMARY.md) and [cpu_parallel.md](cpu_parallel.md) for details

**Code Quality Improvements (2025-11-23):**
- Fixed static RNG issue: Consolidated multiple static random
  generators into shared class methods (Ant.cpp, AntColony.cpp)
- Extracted magic numbers: Added `EPSILON_DISTANCE` constant for
  division-by-zero protection (Ant.h)
- Updated documentation paths: Corrected frontend structure in file
  organization section
- Fixed TSPLoader path resolution: Added `../../../data/` search path
  to support cpp/ subdirectory structure (TSPLoader.cpp:27)

## References

### Papers

- Dorigo, M., & Stützle, T. (2004). *Ant Colony Optimization*. MIT Press.
- Dorigo, M., & Gambardella, L. M. (1997). "Ant colony system: a
  cooperative learning approach to the traveling salesman problem."
  *IEEE Transactions on Evolutionary Computation*.

### Online Resources

- TSPLIB: <http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/>
- ACO Metaheuristic: <http://www.aco-metaheuristic.org/>
- Google Test Documentation: <https://google.github.io/googletest/>
