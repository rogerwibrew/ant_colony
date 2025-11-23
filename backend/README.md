# ACO TSP Solver - Flask Backend

Flask API with WebSocket support for real-time Ant Colony Optimization solving.

## Features

- **REST API** - List benchmarks, get parameters, health checks
- **WebSocket** - Real-time progress updates during optimization
- **C++ Integration** - Direct binding to high-performance C++ solver via pybind11
- **CORS Support** - Ready for Next.js frontend integration

## Installation

### Prerequisites

Ensure Python bindings are built first:

```bash
cd ../python_bindings
pip install pybind11
python setup.py build_ext --inplace
```

### Install Dependencies

```bash
cd backend
pip install -r requirements.txt
```

## Running the Server

```bash
python app.py
```

Server will start on `http://0.0.0.0:5000`

Output:
```
============================================================
ACO TSP Solver - Flask Backend with WebSocket Support
============================================================
CORS origins: ['http://localhost:3000']
Data directory: /home/roger/dev/ant_colony/data
Debug mode: True
Async mode: threading
============================================================
REST Endpoints:
  GET  /api/health
  GET  /api/benchmarks
  GET  /api/benchmarks/<name>
  GET  /api/parameters

WebSocket Events:
  connect -> connected
  preview -> preview_loaded (load cities without solving)
  solve -> loaded, progress (every 10 iter), complete
  cancel -> cancelled
  disconnect
============================================================

Server starting on http://0.0.0.0:5000
```

## Testing

Run the test suite (requires server to be running):

```bash
# Terminal 1: Start server
python app.py

# Terminal 2: Run tests
python test_client.py
```

Expected output:
```
============================================================
ACO Solver Flask API - Test Suite
============================================================

✓ Testing REST Endpoints
✓ Testing WebSocket Solve
✓ ALL TESTS PASSED
============================================================
```

## API Reference

### REST Endpoints

#### GET /api/health

Health check endpoint.

**Response:**
```json
{
  "status": "healthy",
  "service": "ACO TSP Solver API",
  "version": "1.0.0"
}
```

#### GET /api/benchmarks

List all available TSPLIB benchmarks.

**Response:**
```json
{
  "benchmarks": [
    {
      "name": "berlin52.tsp",
      "cities": 52,
      "optimal": 7542
    },
    ...
  ],
  "count": 74
}
```

#### GET /api/benchmarks/<name>

Get information about a specific benchmark.

**Example:** GET `/api/benchmarks/berlin52.tsp`

**Response:**
```json
{
  "name": "berlin52.tsp",
  "cities": 52,
  "optimal": 7542,
  "filepath": "/path/to/data/berlin52.tsp"
}
```

#### GET /api/parameters

Get default ACO parameters.

**Response:**
```json
{
  "parameters": {
    "numAnts": 20,
    "iterations": 100,
    "alpha": 1.0,
    "beta": 2.0,
    "rho": 0.5,
    "Q": 100.0
  },
  "descriptions": {
    "numAnts": "Number of ants in the colony",
    ...
  }
}
```

### WebSocket Events

#### Client → Server

##### `preview`

Load cities for a TSP problem without solving (preview mode).

**Payload:**
```json
{
  "benchmark": "berlin52.tsp"
}
```

##### `solve`

Start solving a TSP problem.

**Payload:**
```json
{
  "benchmark": "berlin52.tsp",
  "params": {
    "numAnts": 20,          // null for auto-calculate (1-2 per city)
    "iterations": 100,
    "alpha": 1.0,
    "beta": 2.0,
    "rho": 0.5,
    "Q": 100.0,
    "useConvergence": false,  // true to use convergence mode
    "convergenceIterations": 200,  // stop after N iterations without improvement
    "useParallel": true,    // enable OpenMP multi-threading
    "numThreads": 0,        // 0=auto-detect, 1=serial, 2+=specific count
    "useLocalSearch": false, // enable 2-opt/3-opt local search
    "use3Opt": true,        // use both 2-opt and 3-opt when LS enabled
    "localSearchMode": "best" // when to apply: "best", "all", or "none"
  }
}
```

##### `cancel`

Cancel running optimization.

**Payload:** None

#### Server → Client

##### `connected`

Sent when client connects.

**Payload:**
```json
{
  "message": "Connected to ACO TSP Solver",
  "version": "1.0.0"
}
```

##### `preview_loaded`

Sent after preview request completes (cities loaded without solving).

**Payload:**
```json
{
  "benchmark": "berlin52.tsp",
  "numCities": 52,
  "cities": [[565.0, 575.0], [25.0, 185.0], ...]
}
```

##### `loaded`

Sent after benchmark is loaded successfully for solving.

**Payload:**
```json
{
  "benchmark": "berlin52.tsp",
  "numCities": 52,
  "cities": [[565.0, 575.0], [25.0, 185.0], ...]
}
```

##### `progress`

Sent every 10 iterations during optimization.

**Payload:**
```json
{
  "iteration": 10,
  "bestDistance": 8347.23,
  "bestTour": [0, 15, 23, 8, 42, ...],
  "convergenceHistory": [12450.5, 11200.3, ..., 8347.23],
  "cities": [[565.0, 575.0], ...],
  "elapsedTime": 0.45,
  "progress": 10.0
}
```

##### `complete`

Sent when optimization completes.

**Payload:**
```json
{
  "bestDistance": 7544.37,
  "bestTour": [0, 15, 23, 8, 42, ...],
  "convergenceHistory": [12450.5, ..., 7544.37],
  "cities": [[565.0, 575.0], ...],
  "elapsedTime": 4.52,
  "totalIterations": 100,
  "benchmark": "berlin52.tsp",
  "optimalDistance": 7542,
  "optimalityGap": 0.03
}
```

##### `cancelled`

Sent when optimization is cancelled.

**Payload:**
```json
{
  "message": "Optimization cancelled"
}
```

##### `error`

Sent when an error occurs.

**Payload:**
```json
{
  "message": "Error description"
}
```

## Example Client (Python)

```python
import socketio

sio = socketio.Client()

@sio.on('progress')
def on_progress(data):
    print(f"Iteration {data['iteration']}: {data['bestDistance']:.2f}")

@sio.on('complete')
def on_complete(data):
    print(f"Final: {data['bestDistance']:.2f} in {data['elapsedTime']:.2f}s")

sio.connect('http://localhost:5000')
sio.emit('solve', {
    'benchmark': 'berlin52.tsp',
    'params': {'iterations': 50}
})
sio.wait()
```

## Example Client (JavaScript)

```javascript
import io from 'socket.io-client';

const socket = io('http://localhost:5000');

socket.on('connect', () => {
  console.log('Connected');
});

socket.on('progress', (data) => {
  console.log(`Iteration ${data.iteration}: ${data.bestDistance}`);
});

socket.on('complete', (data) => {
  console.log(`Final: ${data.bestDistance} in ${data.elapsedTime}s`);
});

socket.emit('solve', {
  benchmark: 'berlin52.tsp',
  params: { iterations: 50 }
});
```

## Configuration

Edit `config.py` to customize:

- **CORS_ORIGINS** - Allowed frontend origins (default: `http://localhost:3000`)
- **DATA_DIR** - Path to TSPLIB files (default: `../data/`)
- **DEFAULT_PARAMS** - Default ACO parameters
- **BENCHMARKS** - Benchmark metadata

Environment variables:
- `FLASK_DEBUG` - Enable debug mode (default: `True`)
- `CORS_ORIGINS` - Comma-separated list of allowed origins
- `SECRET_KEY` - Flask secret key (change in production)

## Performance

**Typical Performance (berlin52.tsp, 100 iterations):**
- C++ solver: ~65ms
- Python callback overhead: ~5ms
- WebSocket emit: ~10ms
- Total: ~80ms

**WebSocket Updates:**
- Progress events: Every 10 iterations
- Latency: <20ms per update
- Concurrent clients: Supported (threading mode)

## Architecture

```
┌─────────────┐  WebSocket  ┌──────────────┐  pybind11  ┌───────────┐
│   Client    │◄───────────►│    Flask     │◄──────────►│  C++ ACO  │
│ (Browser)   │             │ + SocketIO   │            │  Solver   │
└─────────────┘             └──────────────┘            └───────────┘
     │                             │                          │
     │ - Real-time viz             │ - WebSocket server       │ - Graph
     │ - Parameter controls        │ - REST endpoints         │ - AntColony
     │ - Progress updates          │ - Progress callbacks     │ - TSPLoader
```

## Next Steps

This backend is ready for integration with:
- **Next.js frontend** - Tour visualization, convergence plots, parameter controls
- **React dashboard** - Multiple solver instances, comparison mode
- **Mobile app** - React Native or similar

## Notes

- **Threading mode** is used for Python 3.13 compatibility (eventlet has issues)
- **Development server** only - use Gunicorn/uWSGI for production
- **Progress callbacks** release GIL during C++ computation for better concurrency
- **74 TSPLIB benchmarks** available (only EUC_2D format)

## Troubleshooting

**Server won't start:**
- Check Python bindings are built: `python -c "import aco_solver"`
- Install dependencies: `pip install -r requirements.txt`

**WebSocket connection fails:**
- Check CORS settings in `config.py`
- Verify server is running on expected port (5000)
- Check firewall rules

**Slow performance:**
- Reduce `iterations` parameter
- Use smaller benchmarks for testing
- Check system load
