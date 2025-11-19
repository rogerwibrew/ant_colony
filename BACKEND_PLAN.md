# Backend Development Plan: Flask API + Next.js Integration

**Status:** Planning Phase
**Branch:** master (serial implementation, no OpenMP)
**Target:** Web-based ACO TSP solver with real-time visualization

---

## Architecture Overview

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│   Next.js       │         │   Flask API      │         │   C++ ACO       │
│   Frontend      │◄───────►│   + SocketIO     │◄───────►│   Solver        │
│                 │WebSocket│                  │pybind11 │                 │
└─────────────────┘         └──────────────────┘         └─────────────────┘
     │                              │                             │
     │ - Tour visualization         │ - WebSocket server          │ - Graph
     │ - Convergence plots          │ - REST endpoints            │ - AntColony
     │ - Parameter controls         │ - Progress callback         │ - TSPLoader
     │ - Real-time updates          │ - JSON serialization        │ - All classes
```

**Communication Flow:**
1. Frontend sends solve request via WebSocket with benchmark name + parameters
2. Flask creates C++ AntColony instance via pybind11
3. C++ solver calls progress callback every 10 iterations
4. Flask emits WebSocket events with progress data (iteration, best tour, convergence)
5. Frontend updates visualizations in real-time
6. Final solution sent when optimization completes

---

## Development Phases

### Phase 1: C++ Progress Callback System

**Goal:** Enable C++ code to report progress during optimization

**Files to Modify:**

#### 1.1 `include/AntColony.h`

Add callback support:

```cpp
#include <functional>

class AntColony {
public:
    // Progress callback type - called every N iterations
    using ProgressCallback = std::function<void(
        int iteration,                      // Current iteration number
        double bestDistance,                // Best distance found so far
        const std::vector<int>& bestTour,   // Best tour sequence
        const std::vector<double>& convergenceHistory  // All iteration best distances
    )>;

    // Existing methods...

    // New method to register callback
    void setProgressCallback(ProgressCallback callback);

    // New method to set callback interval (default: 10)
    void setCallbackInterval(int interval);

private:
    ProgressCallback progressCallback_;
    int callbackInterval_ = 10;

    // Existing members...
};
```

#### 1.2 `src/AntColony.cpp`

Implement callback functionality:

```cpp
void AntColony::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = callback;
}

void AntColony::setCallbackInterval(int interval) {
    callbackInterval_ = interval;
}

// Modify runIteration() or solve() to invoke callback:
void AntColony::runIteration() {
    // ... existing iteration logic ...

    // After finding best tour for this iteration
    if (progressCallback_ && (currentIteration % callbackInterval_ == 0)) {
        progressCallback_(
            currentIteration,
            bestTour_.getDistance(),
            bestTour_.getSequence(),
            iterationBestDistances_
        );
    }
}
```

**Testing:** Verify callback is invoked every 10 iterations in existing tests

---

### Phase 2: pybind11 Python Bindings

**Goal:** Create Python module to access C++ classes

**New Directory:** `python_bindings/`

#### 2.1 `python_bindings/bindings.cpp`

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "City.h"
#include "Graph.h"
#include "Tour.h"
#include "TSPLoader.h"
#include "AntColony.h"

namespace py = pybind11;

PYBIND11_MODULE(aco_solver, m) {
    m.doc() = "Ant Colony Optimization TSP Solver Python Bindings";

    // City class
    py::class_<City>(m, "City")
        .def(py::init<int, double, double>())
        .def("distanceTo", &City::distanceTo)
        .def("getId", &City::getId)
        .def("getX", &City::getX)
        .def("getY", &City::getY);

    // Graph class
    py::class_<Graph>(m, "Graph")
        .def(py::init<const std::vector<City>&>())
        .def("getDistance", &Graph::getDistance)
        .def("getNumCities", &Graph::getNumCities)
        .def("getCity", &Graph::getCity)
        .def("getCities", &Graph::getCities)
        .def("isValid", &Graph::isValid)
        .def("nearestNeighborTourLength", &Graph::nearestNeighborTourLength);

    // Tour class
    py::class_<Tour>(m, "Tour")
        .def(py::init<>())
        .def("getDistance", &Tour::getDistance)
        .def("getSequence", &Tour::getSequence)
        .def("validate", &Tour::validate);

    // TSPLoader class
    py::class_<TSPLoader>(m, "TSPLoader")
        .def(py::init<const std::string&>())
        .def("loadGraph", &TSPLoader::loadGraph);

    // AntColony class with callback support
    py::class_<AntColony>(m, "AntColony")
        .def(py::init<const Graph&, int, double, double, double, double>(),
             py::arg("graph"),
             py::arg("numAnts") = 20,
             py::arg("alpha") = 1.0,
             py::arg("beta") = 2.0,
             py::arg("rho") = 0.5,
             py::arg("Q") = 100.0)
        .def("initialize", &AntColony::initialize)
        .def("solve", &AntColony::solve)
        .def("getBestTour", &AntColony::getBestTour)
        .def("getConvergenceData", &AntColony::getConvergenceData)
        .def("setProgressCallback", &AntColony::setProgressCallback)
        .def("setCallbackInterval", &AntColony::setCallbackInterval);
}
```

**Key Points:**
- `pybind11/stl.h` - Automatic C++ vector ↔ Python list conversion
- `pybind11/functional.h` - Support for std::function callbacks
- GIL handling: pybind11 automatically manages Python Global Interpreter Lock

#### 2.2 `python_bindings/setup.py`

```python
from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import os

# Get C++ source files
cpp_sources = [
    'bindings.cpp',
    '../src/City.cpp',
    '../src/Graph.cpp',
    '../src/Tour.cpp',
    '../src/TSPLoader.cpp',
    '../src/PheromoneMatrix.cpp',
    '../src/Ant.cpp',
    '../src/AntColony.cpp',
]

ext_modules = [
    Pybind11Extension(
        "aco_solver",
        cpp_sources,
        include_dirs=['../include'],
        cxx_std=17,
        extra_compile_args=['-O3'],
    ),
]

setup(
    name="aco_solver",
    version="1.0.0",
    author="Your Name",
    description="Ant Colony Optimization TSP Solver",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
```

#### 2.3 `python_bindings/__init__.py`

```python
"""ACO Solver Python Bindings"""
from .aco_solver import City, Graph, Tour, TSPLoader, AntColony

__all__ = ['City', 'Graph', 'Tour', 'TSPLoader', 'AntColony']
```

**Build Instructions:**
```bash
cd python_bindings
pip install pybind11
python setup.py build_ext --inplace
```

**Testing in Python:**
```python
import aco_solver

# Load TSPLIB file
loader = aco_solver.TSPLoader("../data/berlin52.tsp")
graph = loader.loadGraph()

# Create colony
colony = aco_solver.AntColony(graph, numAnts=20, iterations=100)

# Define callback
def progress_callback(iteration, best_dist, best_tour, convergence):
    print(f"Iteration {iteration}: Best distance = {best_dist}")

colony.setProgressCallback(progress_callback)
colony.setCallbackInterval(10)

# Solve
best_tour = colony.solve(100)
print(f"Final best: {best_tour.getDistance()}")
```

---

### Phase 3: Flask API with WebSocket Support

**New Directory:** `backend/`

#### 3.1 `backend/requirements.txt`

```
Flask==3.0.0
Flask-SocketIO==5.3.5
Flask-CORS==4.0.0
python-socketio==5.10.0
python-engineio==4.8.0
eventlet==0.34.2
```

#### 3.2 `backend/config.py`

```python
import os
from pathlib import Path

class Config:
    """Flask + ACO configuration"""

    # Flask settings
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'dev-secret-key-change-in-production'
    DEBUG = True

    # CORS settings
    CORS_ORIGINS = os.environ.get('CORS_ORIGINS', 'http://localhost:3000').split(',')

    # WebSocket settings
    SOCKETIO_CORS_ALLOWED_ORIGINS = CORS_ORIGINS
    SOCKETIO_ASYNC_MODE = 'eventlet'

    # ACO solver paths
    BASE_DIR = Path(__file__).parent.parent
    DATA_DIR = BASE_DIR / 'data'

    # Default ACO parameters
    DEFAULT_PARAMS = {
        'numAnts': 20,
        'iterations': 100,
        'alpha': 1.0,
        'beta': 2.0,
        'rho': 0.5,
        'Q': 100.0
    }

    # Benchmark metadata (name, optimal distance)
    BENCHMARKS = {
        'ulysses16.tsp': {'cities': 16, 'optimal': 73.99},
        'berlin52.tsp': {'cities': 52, 'optimal': 7542},
        'eil51.tsp': {'cities': 51, 'optimal': 426},
        'eil76.tsp': {'cities': 76, 'optimal': 538},
        'kroA100.tsp': {'cities': 100, 'optimal': 21282},
        # Add more as needed
    }
```

#### 3.3 `backend/solver_manager.py`

```python
"""Bridge between Flask and C++ ACO solver"""
import sys
import time
from pathlib import Path

# Add python_bindings to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'python_bindings'))

import aco_solver
from config import Config

class SolverManager:
    """Manages ACO solver execution with progress callbacks"""

    def __init__(self, socketio):
        self.socketio = socketio
        self.graph = None
        self.cities_coords = []
        self.start_time = None
        self.is_running = False

    def load_benchmark(self, benchmark_name):
        """Load TSPLIB benchmark file"""
        filepath = Config.DATA_DIR / benchmark_name

        if not filepath.exists():
            raise FileNotFoundError(f"Benchmark {benchmark_name} not found")

        loader = aco_solver.TSPLoader(str(filepath))
        self.graph = loader.loadGraph()

        if not self.graph.isValid():
            raise ValueError(f"Failed to load graph from {benchmark_name}")

        # Extract city coordinates for frontend visualization
        self.cities_coords = [
            [city.getX(), city.getY()]
            for city in self.graph.getCities()
        ]

        return {
            'numCities': self.graph.getNumCities(),
            'cities': self.cities_coords
        }

    def solve(self, params):
        """Run ACO solver with progress callbacks"""
        if self.graph is None or not self.graph.isValid():
            raise RuntimeError("No valid graph loaded")

        # Extract parameters
        num_ants = params.get('numAnts', Config.DEFAULT_PARAMS['numAnts'])
        iterations = params.get('iterations', Config.DEFAULT_PARAMS['iterations'])
        alpha = params.get('alpha', Config.DEFAULT_PARAMS['alpha'])
        beta = params.get('beta', Config.DEFAULT_PARAMS['beta'])
        rho = params.get('rho', Config.DEFAULT_PARAMS['rho'])
        Q = params.get('Q', Config.DEFAULT_PARAMS['Q'])

        # Create colony
        colony = aco_solver.AntColony(
            self.graph,
            num_ants,
            alpha,
            beta,
            rho,
            Q
        )

        # Initialize
        colony.initialize()

        # Set up progress callback
        self.start_time = time.time()
        self.is_running = True

        def progress_callback(iteration, best_distance, best_tour, convergence):
            if not self.is_running:
                return

            elapsed = time.time() - self.start_time
            progress_pct = (iteration / iterations) * 100

            # Emit progress update via WebSocket
            self.socketio.emit('progress', {
                'iteration': iteration,
                'bestDistance': best_distance,
                'bestTour': best_tour,
                'convergenceHistory': convergence,
                'cities': self.cities_coords,
                'elapsedTime': round(elapsed, 2),
                'progress': round(progress_pct, 1)
            })

        colony.setProgressCallback(progress_callback)
        colony.setCallbackInterval(10)

        # Solve
        best_tour = colony.solve(iterations)

        # Send final result
        elapsed = time.time() - self.start_time
        self.is_running = False

        return {
            'bestDistance': best_tour.getDistance(),
            'bestTour': best_tour.getSequence(),
            'convergenceHistory': colony.getConvergenceData(),
            'cities': self.cities_coords,
            'elapsedTime': round(elapsed, 2),
            'totalIterations': iterations
        }

    def stop(self):
        """Stop running solver"""
        self.is_running = False
```

#### 3.4 `backend/app.py`

```python
"""Flask API with WebSocket support for ACO TSP Solver"""
from flask import Flask, jsonify, request
from flask_socketio import SocketIO, emit
from flask_cors import CORS
import traceback

from config import Config
from solver_manager import SolverManager

# Initialize Flask app
app = Flask(__name__)
app.config.from_object(Config)

# Enable CORS
CORS(app, resources={r"/api/*": {"origins": Config.CORS_ORIGINS}})

# Initialize SocketIO
socketio = SocketIO(
    app,
    cors_allowed_origins=Config.SOCKETIO_CORS_ALLOWED_ORIGINS,
    async_mode=Config.SOCKETIO_ASYNC_MODE
)

# Global solver manager
solver_manager = SolverManager(socketio)

# ============================================================================
# REST Endpoints
# ============================================================================

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({'status': 'healthy', 'service': 'ACO TSP Solver'})

@app.route('/api/benchmarks', methods=['GET'])
def list_benchmarks():
    """List available TSPLIB benchmark problems"""
    benchmarks = []

    for filename, metadata in Config.BENCHMARKS.items():
        filepath = Config.DATA_DIR / filename
        if filepath.exists():
            benchmarks.append({
                'name': filename,
                'cities': metadata['cities'],
                'optimal': metadata['optimal']
            })

    return jsonify({'benchmarks': benchmarks})

# ============================================================================
# WebSocket Events
# ============================================================================

@socketio.on('connect')
def handle_connect():
    """Client connected"""
    print('Client connected')
    emit('connected', {'message': 'Connected to ACO solver'})

@socketio.on('disconnect')
def handle_disconnect():
    """Client disconnected"""
    print('Client disconnected')
    solver_manager.stop()

@socketio.on('solve')
def handle_solve(data):
    """Start solving TSP problem"""
    try:
        # Extract request data
        benchmark = data.get('benchmark')
        params = data.get('params', {})

        if not benchmark:
            emit('error', {'message': 'No benchmark specified'})
            return

        # Load benchmark
        load_result = solver_manager.load_benchmark(benchmark)

        # Send initial data
        emit('loaded', {
            'benchmark': benchmark,
            'numCities': load_result['numCities'],
            'cities': load_result['cities']
        })

        # Run solver (progress updates sent via callbacks)
        result = solver_manager.solve(params)

        # Send final result
        emit('complete', result)

    except Exception as e:
        print(f"Error in solve: {traceback.format_exc()}")
        emit('error', {'message': str(e)})

@socketio.on('cancel')
def handle_cancel():
    """Cancel running optimization"""
    solver_manager.stop()
    emit('cancelled', {'message': 'Optimization cancelled'})

# ============================================================================
# Run Server
# ============================================================================

if __name__ == '__main__':
    print("=" * 60)
    print("ACO TSP Solver - Flask Backend")
    print("=" * 60)
    print(f"CORS origins: {Config.CORS_ORIGINS}")
    print(f"Data directory: {Config.DATA_DIR}")
    print("=" * 60)

    socketio.run(
        app,
        host='0.0.0.0',
        port=5000,
        debug=Config.DEBUG
    )
```

#### 3.5 `backend/.gitignore`

```
__pycache__/
*.pyc
*.pyo
*.pyd
.env
venv/
*.log
```

---

### Phase 4: JSON Schema & API Documentation

#### Request: WebSocket 'solve' Event

```json
{
  "benchmark": "berlin52.tsp",
  "params": {
    "numAnts": 20,
    "iterations": 100,
    "alpha": 1.0,
    "beta": 2.0,
    "rho": 0.5,
    "Q": 100.0
  }
}
```

#### Response: WebSocket 'loaded' Event

```json
{
  "benchmark": "berlin52.tsp",
  "numCities": 52,
  "cities": [
    [565.0, 575.0],
    [25.0, 185.0],
    ...
  ]
}
```

#### Response: WebSocket 'progress' Event (Every 10 Iterations)

```json
{
  "iteration": 10,
  "bestDistance": 8234.56,
  "bestTour": [0, 15, 23, 8, 42, ..., 0],
  "convergenceHistory": [12450.5, 11200.3, 10150.8, ..., 8234.56],
  "cities": [[565.0, 575.0], [25.0, 185.0], ...],
  "elapsedTime": 0.45,
  "progress": 10.0
}
```

#### Response: WebSocket 'complete' Event

```json
{
  "bestDistance": 7906.23,
  "bestTour": [0, 15, 23, 8, 42, ..., 0],
  "convergenceHistory": [12450.5, 11200.3, ..., 7906.23],
  "cities": [[565.0, 575.0], [25.0, 185.0], ...],
  "elapsedTime": 4.52,
  "totalIterations": 100
}
```

#### Response: GET /api/benchmarks

```json
{
  "benchmarks": [
    {
      "name": "ulysses16.tsp",
      "cities": 16,
      "optimal": 73.99
    },
    {
      "name": "berlin52.tsp",
      "cities": 52,
      "optimal": 7542
    },
    ...
  ]
}
```

---

## Installation & Setup

### 1. Build pybind11 Module

```bash
# Install pybind11
pip install pybind11

# Build C++ Python module
cd python_bindings
python setup.py build_ext --inplace

# Test in Python
python -c "import aco_solver; print('Success!')"
```

### 2. Install Flask Dependencies

```bash
cd backend
pip install -r requirements.txt
```

### 3. Run Flask Server

```bash
cd backend
python app.py
```

Server runs on `http://localhost:5000`

### 4. Test WebSocket Connection

Create `test_client.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <title>ACO Test Client</title>
    <script src="https://cdn.socket.io/4.5.4/socket.io.min.js"></script>
</head>
<body>
    <h1>ACO Solver Test Client</h1>
    <button onclick="solve()">Solve Berlin52</button>
    <pre id="output"></pre>

    <script>
        const socket = io('http://localhost:5000');

        socket.on('connect', () => {
            console.log('Connected to server');
        });

        socket.on('loaded', (data) => {
            console.log('Loaded:', data);
            document.getElementById('output').textContent +=
                `Loaded ${data.benchmark} (${data.numCities} cities)\n`;
        });

        socket.on('progress', (data) => {
            console.log('Progress:', data);
            document.getElementById('output').textContent +=
                `Iteration ${data.iteration}: ${data.bestDistance.toFixed(2)}\n`;
        });

        socket.on('complete', (data) => {
            console.log('Complete:', data);
            document.getElementById('output').textContent +=
                `\nFinal: ${data.bestDistance.toFixed(2)} in ${data.elapsedTime}s\n`;
        });

        function solve() {
            socket.emit('solve', {
                benchmark: 'berlin52.tsp',
                params: {
                    numAnts: 20,
                    iterations: 100,
                    alpha: 1.0,
                    beta: 2.0,
                    rho: 0.5,
                    Q: 100.0
                }
            });
        }
    </script>
</body>
</html>
```

---

## Testing Strategy

### Unit Tests

1. **C++ Callback Tests:**
   - Verify callback invoked at correct intervals
   - Verify callback receives correct data
   - Test with and without callback set

2. **pybind11 Binding Tests:**
   - Test all exposed classes in Python
   - Verify vector ↔ list conversions
   - Test callback from Python functions
   - Memory leak tests

3. **Flask API Tests:**
   - Test /api/benchmarks endpoint
   - Test WebSocket connection/disconnection
   - Test solve event with valid/invalid data
   - Test concurrent connections (future)

### Integration Tests

1. Load benchmark via Flask → verify C++ loads file
2. Start solve → verify progress events received
3. Complete optimization → verify final result matches C++ CLI
4. Compare solution quality: Flask API vs C++ CLI (should be identical)

---

## Directory Structure After Implementation

```
ant_colony/
├── include/               # C++ headers (modified: AntColony.h)
├── src/                   # C++ source (modified: AntColony.cpp)
├── tests/                 # C++ unit tests
├── data/                  # TSPLIB benchmark files
├── python_bindings/       # NEW: pybind11 module
│   ├── bindings.cpp       # C++ → Python bindings
│   ├── setup.py           # Build configuration
│   ├── __init__.py        # Python package
│   └── aco_solver.*.so    # Compiled module (generated)
├── backend/               # NEW: Flask API
│   ├── app.py             # Flask + SocketIO server
│   ├── solver_manager.py  # C++ ↔ Flask bridge
│   ├── config.py          # Configuration
│   ├── requirements.txt   # Python dependencies
│   └── .gitignore
├── CMakeLists.txt         # C++ build (unchanged)
├── CLAUDE.md              # C++ implementation docs
├── BACKEND_PLAN.md        # This file
└── README.md              # Project overview
```

---

## Next Steps for Frontend (Next.js)

Once backend is complete, frontend can:

1. **Connect to WebSocket:** `socket.io-client` library
2. **Send solve requests:** Benchmark selection + parameter controls
3. **Receive real-time updates:** Update visualizations every 10 iterations
4. **Visualize tour:** Canvas or SVG rendering of city positions + tour path
5. **Plot convergence:** Chart.js or Recharts for iteration vs distance
6. **Display metrics:** Current iteration, best distance, elapsed time, progress bar

**Example Frontend Tech Stack:**
- Next.js 14 (App Router)
- TypeScript
- socket.io-client
- Chart.js or Recharts (convergence plot)
- Canvas API or D3.js (tour visualization)
- Tailwind CSS (styling)

---

## Performance Considerations

### Current (Serial) Implementation:
- berlin52 (52 cities): ~361ms for 100 iterations
- eil76 (76 cities): ~687ms for 100 iterations
- kroA100 (100 cities): ~1143ms for 100 iterations

**Latency Budget:**
- C++ computation: 350-1000ms
- Python callback overhead: ~1-5ms per callback
- WebSocket emit: ~1-10ms
- Network latency: ~10-50ms
- Frontend rendering: ~5-20ms

**Total per update:** ~15-85ms overhead on top of C++ computation

### Future: Adding OpenMP Parallelization

When ready to integrate `openmp-parallel` branch:
1. Merge parallelization code from openmp-parallel → master
2. Rebuild pybind11 module with OpenMP flags
3. 10-12× speedup on multi-core systems
4. No changes needed to Flask API or frontend

---

## Known Limitations & Future Work

### Current Limitations:
1. **Single-user only:** No job queue, one optimization at a time
2. **No job cancellation:** Cancel event handler stub only
3. **No persistent storage:** Results lost after completion
4. **Serial execution:** Using master branch (no OpenMP)
5. **Pre-loaded benchmarks only:** No custom coordinate upload yet

### Future Enhancements:

**Phase 5:** Advanced Features
- Job queue for multiple concurrent users
- Job persistence (SQLite or PostgreSQL)
- Custom problem upload (coordinates or TSPLIB files)
- Export results (CSV, JSON download)
- Comparison mode (run multiple parameter sets)

**Phase 6:** Performance Optimization
- Merge OpenMP parallelization from openmp-parallel branch
- 10-12× faster on multi-core servers
- Adjust callback interval dynamically (more frequent for fast runs)

**Phase 7:** Algorithm Variants
- Elitist ACO (only best ants deposit pheromones)
- MAX-MIN Ant System (enforce pheromone bounds)
- Ant Colony System (ACS) variant
- 2-opt local search for solution improvement

**Phase 8:** Advanced Visualization
- Pheromone matrix heatmap
- Individual ant paths animation
- Parameter sensitivity analysis
- A/B comparison of different configurations

---

## Success Criteria

Backend is complete when:
- ✅ pybind11 module compiles and imports in Python
- ✅ All C++ classes accessible from Python
- ✅ Progress callback works every 10 iterations
- ✅ Flask server starts without errors
- ✅ GET /api/benchmarks returns valid data
- ✅ WebSocket solve event triggers optimization
- ✅ Progress events received by client in real-time
- ✅ Final solution matches C++ CLI output
- ✅ Test client successfully visualizes updates

---

## References

**pybind11:**
- Documentation: https://pybind11.readthedocs.io/
- STL containers: https://pybind11.readthedocs.io/en/stable/advanced/cast/stl.html
- Callbacks: https://pybind11.readthedocs.io/en/stable/advanced/cast/functional.html

**Flask-SocketIO:**
- Documentation: https://flask-socketio.readthedocs.io/
- Client-Server Events: https://socket.io/docs/v4/emitting-events/

**Next.js Integration:**
- socket.io-client: https://socket.io/docs/v4/client-api/
- Chart.js: https://www.chartjs.org/
- Canvas Tour Rendering: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API

---

**Last Updated:** 2025-11-19
**Status:** Ready for implementation
