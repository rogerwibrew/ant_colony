# Ant Colony Optimization - TSP Solver

A high-performance C++ implementation of Ant Colony Optimization algorithm
for solving the Travelling Salesman Problem, with OpenMP multi-threading,
Python bindings, Flask API, and Next.js web interface.

**Performance:** 10-12× speedup on multi-core CPUs (tested on 32 cores)

## Project Structure

```text
ant_colony/
├── cpp/                  # C++ core implementation
│   ├── include/          # C++ header files (.h)
│   ├── src/              # C++ source files (.cpp)
│   ├── tests/            # Google Test files
│   ├── build/            # CMake build directory
│   └── CMakeLists.txt    # CMake configuration
├── data/                 # TSPLIB benchmark instances (113+ files, shared)
├── python_bindings/      # pybind11 Python bindings
├── backend/              # Flask API with WebSocket support
├── frontend/             # Next.js web interface
├── Makefile              # Development commands (start/stop servers)
└── CLAUDE.md             # Class specifications and design
```

## Quick Start

### Option 1: Command Line (C++)

```bash
# Build
cd cpp
mkdir build && cd build
cmake .. && cmake --build .

# Run
./bin/ant_colony_tsp berlin52.tsp
```

### Option 2: Web Interface (Recommended)

**Using Makefile (easiest):**

```bash
# Install dependencies (first time only)
make install

# Start both frontend and backend
make start

# Open http://localhost:3000

# Stop servers when done
make stop
```

**Manual setup:**

```bash
# Terminal 1: Start backend
cd backend
pip install -r requirements.txt
python app.py

# Terminal 2: Start frontend
cd frontend
npm install
npm run dev

# Open http://localhost:3000
```

**For remote/headless servers:**

Add to your local `~/.ssh/config`:

```text
Host your-server
    HostName your-server-address
    User your-username
    LocalForward 3000 localhost:3000
    LocalForward 5000 localhost:5000
```

Then: `ssh your-server`, run `make start`, and access
http://localhost:3000 locally.

## Components

### C++ Core (126 tests passing)

High-performance ACO implementation with:

- **OpenMP multi-threading** (10-12× speedup on multi-core CPUs)
- **2-opt/3-opt local search** (achieving 0.03% above optimal on berlin52)
- Precomputed O(1) distance matrix lookups
- TSPLIB format support (EUC_2D)
- Convergence tracking and progress callbacks
- CLI with customizable parameters

```bash
# With local search (recommended for best quality)
./ant_colony_tsp berlin52.tsp --local-search --ants 30 --iterations 100

# Multi-threaded (default - auto-detects cores)
./ant_colony_tsp berlin52.tsp --ants 50 --iterations 200

# Use specific number of threads
./ant_colony_tsp berlin52.tsp --threads 8

# Force single-threaded execution
./ant_colony_tsp berlin52.tsp --serial

# 2-opt only (faster than 2-opt+3-opt)
./ant_colony_tsp berlin52.tsp --local-search --2opt-only

# Full parameter customization
./ant_colony_tsp berlin52.tsp --ants 50 --iterations 200 --alpha 1.5 --beta 3.0 --threads 16 --local-search
```

### Python Bindings

pybind11 bindings exposing the C++ solver to Python with full OpenMP and local search support:

```python
import aco_solver

# Load problem
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()

# Create colony
colony = aco_solver.AntColony(graph, 30, 1.0, 2.0, 0.5, 100.0)

# Control threading (optional - defaults to multi-threaded)
colony.setUseParallel(True)   # Enable OpenMP
colony.setNumThreads(0)        # 0=auto-detect, 1=serial, 2+=specific count

# Enable local search for better solution quality
colony.setUseLocalSearch(True)      # Enable 2-opt/3-opt
colony.setUse3Opt(True)              # Use both 2-opt and 3-opt (default)
colony.setLocalSearchMode("best")    # Apply to best tour only (default)

# Solve
best_tour = colony.solve(100)
print(f"Best distance: {best_tour.getDistance():.2f}")
```

### Flask Backend

REST API + WebSocket server for real-time optimization:

- `GET /api/benchmarks` - List available problems
- `GET /api/health` - Health check
- WebSocket `solve` event - Run optimization with progress updates

### Next.js Frontend

React-based web interface featuring:

- Problem selection from TSPLIB benchmarks
- **Solver type selection** (Single-threaded / Multi-threaded / GPU)
- **Local search controls** (Enable/disable, 2-opt only, mode selection)
- Real-time city/tour visualization with dynamic scaling
- Live iteration path updates (updates every 10 iterations)
- Convergence chart with auto-scaling y-axis
- Best path visualization with solution quality metrics
- Parameter configuration (alpha, beta, rho, iterations, ants)
- Convergence-based stopping criterion
- **Status console** showing iteration progress and optimality gap
- Clean cream/blue color scheme
- Responsive layout with 2x2 grid dashboard

## Build Instructions

### Prerequisites

- CMake 3.14+, C++17 compiler with OpenMP support (GCC, Clang, MSVC)
- Python 3.8+ (for bindings/backend)
- Node.js 18+ (for frontend)

**Note:** OpenMP is optional. If unavailable, the code gracefully falls back
to serial execution. Most modern compilers include OpenMP by default.

### C++ Build

```bash
cd cpp
mkdir build && cd build
cmake ..    # Should show "OpenMP found - parallel execution enabled"
cmake --build .

# Run tests (126 tests)
ctest --output-on-failure

# Test CLI with threading and local search
./bin/ant_colony_tsp berlin52.tsp                    # Auto-detect cores
./bin/ant_colony_tsp berlin52.tsp --local-search     # With local search
./bin/ant_colony_tsp berlin52.tsp --threads 8        # Use 8 threads
./bin/ant_colony_tsp berlin52.tsp --serial           # Single-threaded
```

### Python Bindings

```bash
cd python_bindings
pip install -e .
```

### Backend

```bash
cd backend
pip install -r requirements.txt
python app.py  # Runs on http://localhost:5000
```

### Frontend

```bash
cd frontend
npm install
npm run dev   # Runs on http://localhost:3000
```

## Algorithm Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| alpha     | 1.0     | Pheromone importance |
| beta      | 2.0     | Heuristic (distance) importance |
| rho       | 0.5     | Evaporation rate |
| Q         | 100.0   | Pheromone deposit factor |
| numAnts   | 20      | Number of ants |
| iterations| 100     | Maximum iterations |
| useParallel | true  | Enable OpenMP multi-threading |
| numThreads | 0      | Thread count (0=auto, 1=serial, 2+=specific) |
| useLocalSearch | false | Enable 2-opt/3-opt local search |
| use3Opt | true    | Use both 2-opt and 3-opt (when local search enabled) |
| localSearchMode | best | When to apply local search (best, all, none) |

## Benchmark Results

### Solution Quality (with Local Search)

| Problem | Cities | Optimal | ACO + LS Result | Gap |
|---------|--------|---------|-----------------|-----|
| berlin52| 52     | 7542    | 7544-7607       | 0.03-0.86% |
| eil51   | 51     | 426     | 442-448         | 3.8-5.2% |
| st70    | 70     | 675     | 699-724         | 3.5-7.3% |

**Best recorded result:** berlin52 at 7544.37 (0.03% above optimal)

See [BENCHMARKS.md](BENCHMARKS.md) for comprehensive benchmarking details.

### Performance (OpenMP Multi-Threading)

Tested on 32-core CPU with 30 ants, 100 iterations:

| Problem | Serial Time | Multi-threaded (32 cores) | Speedup |
|---------|-------------|---------------------------|---------|
| berlin52| 361ms       | 35ms                      | 10.3×   |
| eil76   | 687ms       | 61ms                      | 11.3×   |
| kroA100 | 1143ms      | 99ms                      | 11.6×   |

**Note:** Speedup scales with core count. Performance gains are near-linear
up to 8 cores, with diminishing returns at higher core counts due to
synchronization overhead and Amdahl's Law.

## Documentation

For detailed information, see:

- **[CLAUDE.md](CLAUDE.md)** - Complete class specifications, architecture,
  and development guide
- **[BENCHMARKS.md](BENCHMARKS.md)** - Comprehensive local search benchmarking
  results and analysis
- **[cpu_parallel.md](cpu_parallel.md)** - In-depth explanation of OpenMP
  parallelization (educational guide)
- **[INTEGRATION_SUMMARY.md](INTEGRATION_SUMMARY.md)** - Full-stack OpenMP
  integration details

## License

[Add your license here]
