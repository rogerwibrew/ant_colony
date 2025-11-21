# Ant Colony Optimization - TSP Solver

A C++ implementation of Ant Colony Optimization algorithm for solving the Travelling Salesman Problem, with Python bindings, Flask API, and Next.js web interface.

## Project Structure

```
ant_colony/
├── include/              # C++ header files (.h)
├── src/                  # C++ source files (.cpp)
├── tests/                # Google Test files
├── data/                 # TSPLIB benchmark instances (113+ files)
├── python_bindings/      # pybind11 Python bindings
├── backend/              # Flask API with WebSocket support
├── frontend/             # Next.js web interface
├── CMakeLists.txt        # CMake configuration
└── CLAUDE.md             # Class specifications and design
```

## Quick Start

### Option 1: Command Line (C++)

```bash
# Build
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
```
Host your-server
    HostName your-server-address
    User your-username
    LocalForward 3000 localhost:3000
    LocalForward 5000 localhost:5000
```

Then: `ssh your-server`, run `make start`, and access http://localhost:3000 locally.

## Components

### C++ Core (106 tests passing)

High-performance ACO implementation with:
- Precomputed O(1) distance matrix lookups
- TSPLIB format support (EUC_2D)
- Convergence tracking and progress callbacks
- CLI with customizable parameters

```bash
./ant_colony_tsp berlin52.tsp --ants 50 --iterations 200 --alpha 1.5 --beta 3.0
```

### Python Bindings

pybind11 bindings exposing the C++ solver to Python:

```python
import aco_solver
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()
colony = aco_solver.AntColony(graph, 20, 1.0, 2.0, 0.5, 100.0)
best_tour = colony.solve(100)
print(f"Best distance: {best_tour.getDistance()}")
```

### Flask Backend

REST API + WebSocket server for real-time optimization:
- `GET /api/benchmarks` - List available problems
- `GET /api/health` - Health check
- WebSocket `solve` event - Run optimization with progress updates

### Next.js Frontend

React-based web interface featuring:
- Problem selection from TSPLIB benchmarks
- Real-time city/tour visualization with dynamic scaling
- Live iteration path updates (updates every 10 iterations)
- Convergence chart with auto-scaling y-axis
- Best path visualization
- Parameter configuration (alpha, beta, rho)
- Clean cream/blue color scheme
- Responsive layout with 2x2 grid dashboard

## Build Instructions

### Prerequisites

- CMake 3.14+, C++17 compiler
- Python 3.8+ (for bindings/backend)
- Node.js 18+ (for frontend)

### C++ Build

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Run tests
ctest --output-on-failure
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

## Benchmark Results

| Problem | Cities | Optimal | ACO Result | Gap |
|---------|--------|---------|------------|-----|
| berlin52| 52     | 7542    | ~7630      | ~1.2% |
| eil51   | 51     | 426     | ~432       | ~1.4% |
| kroA100 | 100    | 21282   | ~21800     | ~2.4% |

## License

[Add your license here]
