# pybind11: Connecting C++ to Python

## The Problem

We have a fast C++ ACO solver, but we want to:
- Call it from Python (for the Flask backend)
- Use Python's easy web frameworks
- Keep C++ performance for the heavy computation

**Solution:** pybind11 creates a bridge between C++ and Python.

## How It Works

```
┌─────────────────┐    pybind11     ┌─────────────────┐
│                 │    creates      │                 │
│   C++ Code      │ ────────────>   │  Python Module  │
│   (.cpp, .h)    │                 │  (.so file)     │
│                 │                 │                 │
└─────────────────┘                 └─────────────────┘
                                            │
                                            │ import aco_solver
                                            ▼
                                    ┌─────────────────┐
                                    │  Python Code    │
                                    │  (Flask app)    │
                                    └─────────────────┘
```

## The Key File: `bindings.cpp`

Located at `python_bindings/bindings.cpp`, this file tells pybind11 what to expose:

```cpp
#include <pybind11/pybind11.h>
#include "City.h"
#include "Graph.h"
#include "AntColony.h"

namespace py = pybind11;

PYBIND11_MODULE(aco_solver, m) {
    // Now define what Python can access...
    
    // Expose the City class
    py::class_<City>(m, "City")
        .def(py::init<int, double, double>())  // Constructor
        .def("getId", &City::getId)            // Method
        .def("getX", &City::getX)
        .def("getY", &City::getY);
    
    // Expose the Graph class
    py::class_<Graph>(m, "Graph")
        .def("getNumCities", &Graph::getNumCities)
        .def("getDistance", &Graph::getDistance)
        .def("getCities", &Graph::getCities);
    
    // Expose the AntColony class
    py::class_<AntColony>(m, "AntColony")
        .def(py::init<const Graph&, int, double, double, double, double>())
        .def("solve", &AntColony::solve)
        .def("getBestTour", &AntColony::getBestTour);
}
```

## Breaking It Down

### 1. The Module Definition

```cpp
PYBIND11_MODULE(aco_solver, m) {
    // aco_solver = name you'll use in Python: import aco_solver
    // m = the module object we're building
}
```

### 2. Exposing a Class

```cpp
py::class_<City>(m, "City")
//          ^C++ class    ^Python name
```

### 3. Exposing a Constructor

```cpp
.def(py::init<int, double, double>())
//            ^argument types
```

This lets Python do: `city = aco_solver.City(1, 100.0, 200.0)`

### 4. Exposing Methods

```cpp
.def("getX", &City::getX)
//   ^Python name  ^C++ method pointer
```

This lets Python do: `x = city.getX()`

## Using It in Python

After building, you can do:

```python
import aco_solver

# Create objects (calls C++ constructors)
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()

# Call methods (calls C++ methods)
print(f"Cities: {graph.getNumCities()}")

# Create the colony (calls C++ constructor)
colony = aco_solver.AntColony(
    graph,      # C++ Graph object
    20,         # numAnts
    1.0,        # alpha
    2.0,        # beta
    0.5,        # rho
    100.0       # Q
)

# Run the solver (calls C++ solve method)
best_tour = colony.solve(100)  # 100 iterations

# Get results (calls C++ methods)
print(f"Best distance: {best_tour.getDistance()}")
print(f"Tour: {best_tour.getSequence()}")
```

## The Build Process

### Step 1: CMake Configuration

`python_bindings/CMakeLists.txt`:
```cmake
# Find pybind11
find_package(pybind11 REQUIRED)

# Create the Python module
pybind11_add_module(aco_solver bindings.cpp)

# Link with our C++ library
target_link_libraries(aco_solver PRIVATE ant_colony_lib)
```

### Step 2: Build

```bash
cd python_bindings
pip install -e .  # Builds and installs the module
```

### Step 3: Result

Creates: `aco_solver.cpython-312-x86_64-linux-gnu.so`
- `.so` = shared object (Linux library)
- `cpython-312` = Python 3.12
- This file IS the Python module

## Files in python_bindings/

```
python_bindings/
├── bindings.cpp      # The pybind11 bridge definitions
├── CMakeLists.txt    # Build configuration
├── setup.py          # Python package setup
└── README.md         # Documentation
```

## Advanced: Callbacks

We use callbacks so C++ can notify Python during solving:

### C++ Side

```cpp
// In AntColony class
using ProgressCallback = std::function<void(int, double, std::vector<int>, std::vector<double>)>;

void setProgressCallback(ProgressCallback cb) {
    progressCallback_ = cb;
}

// During solve(), call the callback
if (progressCallback_) {
    progressCallback_(iteration, bestDistance, bestTour, convergence);
}
```

### Binding

```cpp
.def("setProgressCallback", &AntColony::setProgressCallback)
```

### Python Side

```python
def on_progress(iteration, best_distance, best_tour, convergence):
    print(f"Iteration {iteration}: {best_distance}")
    # Send WebSocket event to frontend
    socketio.emit('progress', {...})

colony.setProgressCallback(on_progress)
colony.solve(100)  # on_progress gets called every N iterations
```

## Why Not Just Use Python?

| Approach | Performance | Ease of Use |
|----------|-------------|-------------|
| Pure Python | Slow (100x slower) | Easy |
| Pure C++ | Fast | No web frameworks |
| C++ with pybind11 | Fast + Python integration | Best of both |

**Our approach:**
- C++ for the heavy computation (ACO algorithm)
- Python for web server, APIs, convenience
- pybind11 bridges them seamlessly

## Common Issues

### Import Error
```
ImportError: cannot import name 'aco_solver'
```
**Fix:** Make sure you built and installed: `pip install -e .`

### Symbol Not Found
```
undefined symbol: _ZN4City5getIdEv
```
**Fix:** Rebuild after C++ changes: `pip install -e . --force-reinstall`

### Version Mismatch
```
Python version mismatch
```
**Fix:** Use same Python version for building and running
