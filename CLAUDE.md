# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

C++17 Ant Colony Optimization (ACO) implementation for Travelling Salesman Problem using CMake and Google Test.

**Current Status:** ✅ **FULLY IMPLEMENTED** - 106 tests passing. All core classes complete and working. Production-ready ACO solver with CLI interface and TSPLIB format support.

## Roadmap / Planned Tasks

1. **Fix UI** - Improve the web interface
2. **Multi-core CPU support** - Integrate parallel processing using multiple CPU cores (separate branch)
3. **GPU acceleration** - Integrate GPU operations for performance
4. **Additional solution methods** - Add elite ant and other ACO variants
5. **Full TSPLIB95 support** - Add all TSPLIB95 problems to the potential list, including fixing incompatible files in `uncompatibleData/`

## Essential Commands

### Build

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

**Note:** The project includes a `.clangd` configuration file for proper C++17 support and strict include checking.

### Test

```bash
# Run all tests (106 passing)
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

# Show help
./ant_colony_tsp --help

# Works from any directory - files automatically found in data/
cd cpp/build && ./bin/ant_colony_tsp berlin52.tsp
```

**Note:** TSPLoader automatically searches for files in `data/`, `../data/`, and `../../data/`, so you can run from any directory without specifying the full path.

## Architecture Overview

**Implementation Status:**

- ✅ **City** (7 tests) - Represents cities with coordinates and distance calculation
- ✅ **Graph** (8 tests) - Stores cities + precomputed O(1) distance matrix
- ✅ **TSPLoader** (10 tests) - Loads TSP files (coordinate, matrix, and TSPLIB formats)
- ✅ **Tour** (16 tests) - Solution representation with validation
- ✅ **PheromoneMatrix** (20 tests) - Pheromone tracking with evaporation and deposit
- ✅ **Ant** (21 tests) - Solution construction agent with probabilistic city selection
- ✅ **AntColony** (23 tests) - Main algorithm coordinator with convergence tracking
- ✅ **main.cpp** - Full CLI with parameter customization and formatted output

**Execution Flow:**

```
Load TSP Instance → Initialize Colony → For each iteration:
  → Ants construct tours (probabilistic city selection)
  → Update pheromones (evaporation + deposit based on tour quality)
  → Track best solution
→ Output best tour found + convergence data
```

**Critical Design Details:**

- Distance matrix is precomputed once in Graph constructor (O(n²)) for O(1) lookups
- Matrix stored as `std::vector<std::vector<double>>` (runtime-sized, symmetric)
- Initial pheromone value uses **τ₀ = m / C^nn** formula (m = number of ants, C^nn = nearest neighbor tour length)
- Nearest neighbor heuristic provides problem-adaptive initialization
- ACO flow: Ants build tours → Update pheromones → Track best → Repeat
- Member variables use trailing underscore: `id_`, `cities_`
- Single-arg constructors are `explicit`

## File Organization

```
cpp/              - C++ core implementation
  ├── include/    - Header files (.h)
  │   ├── City.h, Graph.h, Tour.h
  │   ├── PheromoneMatrix.h, Ant.h, AntColony.h
  │   └── TSPLoader.h
  ├── src/        - Implementation (.cpp), main.cpp excluded from tests
  │   ├── City.cpp, Graph.cpp, Tour.cpp
  │   ├── PheromoneMatrix.cpp, Ant.cpp, AntColony.cpp
  │   ├── TSPLoader.cpp
  │   └── main.cpp
  ├── tests/      - Google Test files (*_test.cpp)
  │   ├── City_test.cpp, Graph_test.cpp, Tour_test.cpp
  │   ├── PheromoneMatrix_test.cpp, Ant_test.cpp, AntColony_test.cpp
  │   ├── TSPLoader_test.cpp, example_test.cpp
  │   └── data/   - Test input files (simple_5.txt, triangle_3.txt, matrix_4.txt, invalid.txt)
  ├── build/      - CMake build directory (gitignored)
  │   └── bin/    - Compiled executables (ant_colony_tsp, ant_colony_tests)
  └── CMakeLists.txt
data/             - TSPLIB benchmark instances (113+ files, shared by all components)
  ├── berlin52.tsp, ulysses16.tsp, att48.tsp, eil51.tsp
  └── ... (many more EUC_2D problems)
python_bindings/  - pybind11 Python bindings
backend/          - Flask API with WebSocket support
frontend/         - Next.js web interface
```

**Note:** `compile_commands.json` in the root is a symlink to `cpp/build/compile_commands.json` (gitignored but useful for LSP).

## Adding New Classes

1. Create `cpp/include/ClassName.h` and `cpp/src/ClassName.cpp`
2. Add `cpp/tests/ClassName_test.cpp`
3. CMake auto-discovers via GLOB_RECURSE - just rebuild

## Input File Formats

**Simple Coordinate Format:**

```
5                    # Number of cities
0 10.0 20.0         # City ID, X coordinate, Y coordinate
1 15.0 25.0
2 30.0 40.0
3 50.0 10.0
4 20.0 30.0
```

**Simple Distance Matrix Format:**

```
4                    # Number of cities
0.0 5.0 10.0 15.0   # n×n distance matrix
5.0 0.0 8.0 12.0
10.0 8.0 0.0 6.0
15.0 12.0 6.0 0.0
```

**TSPLIB Format (113+ benchmark files in data/):**

```
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

**Selection probability:** `p[i][j] = (pheromone^alpha * (1/distance)^beta) / sum(unvisited)`

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
- `double distanceTo(const City& other) const` - Calculate Euclidean distance to another city
- `int getId() const` - Getter for city ID
- `double getX() const` - Getter for X coordinate
- `double getY() const` - Getter for Y coordinate

**Dependencies:** None (standalone class)

**Notes:**

- Uses Euclidean distance formula: `sqrt((x2-x1)^2 + (y2-y1)^2)`
- Could be extended to support other distance metrics

---

### Class: Graph

**Purpose:** Stores the complete TSP problem instance including all cities and precomputed distances.

**Member Variables:**

- `std::vector<City> cities_` - All cities in the problem
- `std::vector<std::vector<double>> distanceMatrix_` - Precomputed distances between all city pairs
- `int numCities_` - Number of cities

**Methods:**

- `Graph(const std::vector<City>& cities)` - Constructor that builds distance matrix
- `Graph()` - Default constructor (empty graph for error handling)
- `double getDistance(int cityA, int cityB) const` - Get distance between two cities (O(1))
- `int getNumCities() const` - Get total number of cities
- `const City& getCity(int index) const` - Get city by index
- `const std::vector<City>& getCities() const` - Get all cities
- `bool isValid() const` - Check if graph has cities
- `double nearestNeighborTourLength(int startCity = 0) const` - Calculate tour length using greedy nearest neighbor heuristic

**Dependencies:** City

**Notes:**

- Distance matrix is symmetric for Euclidean TSP
- Matrix computed once at construction for efficiency (O(n²) construction, O(1) lookup)
- Nearest neighbor heuristic used to compute initial pheromone value: τ₀ = m / C^nn

---

### Class: Tour

**Purpose:** Represents a complete tour (solution) visiting all cities.

**Member Variables:**

- `std::vector<int> citySequence_` - Ordered list of city IDs
- `double totalDistance_` - Total tour length
- `bool isValid_` - Whether tour visits all cities exactly once

**Methods:**

- `Tour()` - Default constructor
- `Tour(const std::vector<int>& sequence, double distance)` - Constructor with values
- `void setTour(const std::vector<int>& sequence, double distance)` - Set tour data
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

- `std::vector<std::vector<double>> pheromones_` - Pheromone levels between city pairs
- `int numCities_` - Number of cities
- `double initialPheromone_` - Initial pheromone level
- `double minPheromone_` - Minimum allowed pheromone level
- `double maxPheromone_` - Maximum allowed pheromone level (for MAX-MIN Ant System)

**Methods:**

- `PheromoneMatrix(int numCities, double initial)` - Constructor
- `void initialize(double value)` - Reset all pheromones to initial value
- `double getPheromone(int cityA, int cityB) const` - Get pheromone level
- `void setPheromone(int cityA, int cityB, double value)` - Set pheromone level
- `void evaporate(double rho)` - Apply evaporation to all edges: `pheromone *= (1 - rho)`
- `void depositPheromone(int cityA, int cityB, double amount)` - Add pheromone to edge
- `void clampPheromones()` - Enforce min/max bounds (optional, for MMAS variant)

**Dependencies:** None

**Notes:**

- Matrix is symmetric for undirected TSP
- Consider implementing bounds for MAX-MIN Ant System variant

---

### Class: Ant

**Purpose:** Individual ant agent that constructs a tour using pheromone and heuristic information.

**Member Variables:**

- `int currentCity_` - Current position
- `std::vector<bool> visited_` - Tracks visited cities
- `std::vector<int> tour_` - Current tour being constructed
- `double tourLength_` - Length of current tour

**Methods:**

- `Ant(int startCity, int numCities)` - Constructor
- `void reset(int startCity)` - Reset ant to start a new tour
- `int selectNextCity(const Graph& graph, const PheromoneMatrix& pheromones, double alpha, double beta)` - Choose next city probabilistically
- `void visitCity(int city, const Graph& graph)` - Add city to tour
- `bool hasVisitedAll() const` - Check if tour is complete
- `Tour completeTour(const Graph& graph)` - Finalize and return the tour
- `double calculateTourLength(const Graph& graph) const` - Calculate total tour distance

**Dependencies:** Graph, PheromoneMatrix, Tour

**Notes:**

- Selection probability: `p_ij = (pheromone^alpha * (1/distance)^beta) / sum(all unvisited)`
- Uses roulette wheel selection for next city
- **Performance critical:** This is the hotspot (~60-70% of runtime)

---

### Class: AntColony

**Purpose:** Manages the colony of ants, pheromone matrix, and algorithm execution.

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

**Methods:**

- `AntColony(const Graph& graph, int numAnts, double alpha, double beta, double rho, double Q)` - Constructor
- `void initialize()` - Initialize pheromones using τ₀ = m / C^nn formula and reset state
- `void runIteration()` - Execute one complete iteration
- `void constructSolutions()` - All ants build tours
- `void updatePheromones()` - Evaporate and deposit pheromones
- `Tour solve(int maxIterations)` - Run algorithm for specified iterations
- `const Tour& getBestTour() const` - Get best solution found
- `const std::vector<double>& getConvergenceData() const` - Get iteration history

**Dependencies:** Graph, PheromoneMatrix, Ant, Tour

**Notes:**

- Initial pheromone computed as τ₀ = m / C^nn (recommended in ACO literature)
- Falls back to τ₀ = 1.0 if nearest neighbor tour length is zero or invalid
- Can implement different pheromone update strategies (ant-cycle, ant-quantity, etc.)
- Consider elitist strategy (only best ant deposits pheromones)

---

### Class: TSPLoader

**Purpose:** Loads TSP problem instances from files (coordinate, matrix, or TSPLIB format).

**Member Variables:**

- `std::string filename_` - Path to problem file

**Methods:**

- `TSPLoader(const std::string& filename)` - Constructor (auto-searches common paths)
- `Graph loadGraph()` - Parse file and return Graph object (auto-detects format)
- `static Graph loadFromCoordinates(const std::string& filename)` - Load from coordinate file
- `static Graph loadFromDistanceMatrix(const std::string& filename)` - Load from distance matrix
- `static Graph loadFromTSPLIB(const std::string& filename)` - Load from TSPLIB format

**Dependencies:** Graph, City

**Notes:**

- Auto-detection: Checks for "NAME:", "TYPE:", "DIMENSION:" (TSPLIB), 3 fields → coordinates, >3 fields → matrix
- Auto-searches paths: current dir, `data/`, `../data/`, `../../data/`
- Returns empty Graph on error (check with `graph.isValid()`)
- TSPLIB: Only EUC_2D edge weight type currently supported

---

## Performance Notes

**Expected Performance Characteristics:**

- **~60-70%**: Probability calculation and city selection in `Ant::selectNextCity()`
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

Use `std::vector<std::vector<double>>` for distance and pheromone matrices even though sizes are fixed after initialization.

**Reasons:**

- Matrix size is only known at runtime, not compile-time
- `std::array` requires compile-time constant size
- `std::vector` has zero overhead for element access once allocated
- Both use contiguous memory with O(1) random access

**Potential Optimization (if profiling shows bottleneck):**

- Flattened 1D vector: `std::vector<double>` with index `[i*n + j]` for better cache locality
- Only store upper triangle of symmetric matrix (saves 50% memory)
- Use `float` instead of `double` if precision allows

## Testing Strategy

**Unit Tests (Google Test) - 106 tests total:**

1. **City Class (7 tests):** Distance calculation accuracy, getter methods, edge cases
2. **Graph Class (8 tests):** Distance matrix symmetry, correct lookups, invalid indices
3. **Tour Class (16 tests):** Tour validation, distance calculation, invalid tour detection, edge cases
4. **PheromoneMatrix Class (20 tests):** Initialization, evaporation, deposit, clamping, symmetry
5. **Ant Class (21 tests):** Tour construction, probabilistic selection, parameter sensitivity, edge cases
6. **AntColony Class (23 tests):** Convergence, best tour tracking, parameter variations, solution quality
7. **TSPLoader Class (10 tests):** Format detection, coordinate loading, matrix loading, error handling
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
./ant_colony_tsp ulysses16.tsp --ants 50 --iterations 300 --alpha 1.5 --beta 3.0

# Smaller problem, faster convergence
./ant_colony_tsp eil51.tsp --ants 20 --iterations 100 --rho 0.3
```

### Example Output

```
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
- kroA150, kroB150 (both 150), pr152 (152), rat195 (195), kroA200, kroB200 (both 200)

**Large (500+ cities):**
- rat575 (575), rat783 (783), pr1002 (1002), u1060 (1060), vm1084 (1084)
- And many more up to 85,900 cities!

## Known Limitations

1. **Edge Weight Types:** Only EUC_2D (Euclidean 2D) is supported. Files with ATT, GEO, or EXPLICIT types will fail to load.

2. **Pheromone Strategy:** Uses basic Ant Cycle (pheromone deposited after tour completion). Does not implement:
   - Ant Quantity (deposit during construction)
   - Elitist strategy (only best ant deposits)
   - MAX-MIN Ant System bounds (implemented but not enforced by default)

3. **Solution Quality:** For large problems (1000+ cities), solutions may be 10-20% above optimal. Better parameter tuning or more iterations may help.

4. **Performance:** No parallelization. All ants run sequentially. Could benefit from multi-threading.

5. **Local Search:** No 2-opt or 3-opt local search improvement. Adding this would significantly improve solution quality.

## Web Interface

The project includes a full-stack web interface for interactive TSP solving.

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
- `city-visualization.tsx` - City/tour visualization
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
- REST API for benchmark listing
- WebSocket server for real-time optimization progress
- Integration with C++ solver via pybind11

**Running:**
```bash
cd backend
pip install -r requirements.txt
python app.py  # http://localhost:5000
```

## Future Enhancements

- Support for ATT, GEO, and EXPLICIT edge weight types
- Elitist pheromone update strategy
- 2-opt/3-opt local search
- Parallel ant execution
- Adaptive parameters (dynamic alpha, beta, rho)
- Improved web UI (better visualization, more controls)
- Export results to file (CSV, JSON)

## References

### Papers

- Dorigo, M., & Stützle, T. (2004). *Ant Colony Optimization*. MIT Press.
- Dorigo, M., & Gambardella, L. M. (1997). "Ant colony system: a cooperative learning approach to the traveling salesman problem." *IEEE Transactions on Evolutionary Computation*.

### Online Resources

- TSPLIB: <http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/>
- ACO Metaheuristic: <http://www.aco-metaheuristic.org/>
- Google Test Documentation: <https://google.github.io/googletest/>
