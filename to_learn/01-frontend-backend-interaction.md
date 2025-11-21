# Frontend-Backend Interaction

## Overview

Our application has two separate programs running:

1. **Frontend** (Next.js) - Runs in your browser, shows the UI
2. **Backend** (Flask) - Runs on your computer, does the actual computation

They communicate over the network, even though both are on your computer.

## How It Works Step-by-Step

### 1. User Opens the Browser

When you visit `http://localhost:3000`:
- Next.js serves HTML/CSS/JavaScript to your browser
- Your browser runs the React code
- The `useSocket` hook establishes a WebSocket connection to `localhost:5000`

### 2. User Clicks "Solve"

```
Browser                              Backend
   │                                    │
   │  1. emit("solve", {                │
   │     benchmark: "berlin52.tsp",     │
   │     params: { alpha: 1, beta: 2 }  │
   │  })                                │
   │ ──────────────────────────────────>│
   │                                    │  2. Flask receives event
   │                                    │  3. Python loads TSP file
   │                                    │  4. Python calls C++ solver
   │                                    │
   │  5. emit("loaded", {               │
   │     cities: [[x,y], ...],          │
   │     numCities: 52                  │
   │  })                                │
   │ <──────────────────────────────────│
   │                                    │
   │  6. emit("progress", {             │  (repeated every 10 iterations)
   │     iteration: 10,                 │
   │     bestDistance: 8500             │
   │  })                                │
   │ <──────────────────────────────────│
   │                                    │
   │  7. emit("complete", {             │
   │     bestDistance: 7630,            │
   │     bestTour: [0,3,1,...]          │
   │  })                                │
   │ <──────────────────────────────────│
```

### 3. Frontend Updates the UI

Each time the frontend receives an event, React re-renders:
- `loaded` → Show the cities on the map
- `progress` → Update the chart, show current best distance
- `complete` → Show final result, enable "Solve" button again

## Key Code Locations

### Frontend: `useSocket.ts`

```typescript
// Connect to backend
const socket = io("http://localhost:5000")

// Listen for events FROM the backend
socket.on("loaded", (data) => {
  setCities(data.cities)  // Update React state → triggers re-render
})

socket.on("progress", (data) => {
  setBestDistance(data.bestDistance)
})

// Send events TO the backend
socket.emit("solve", { benchmark: "berlin52.tsp", params: {...} })
```

### Backend: `app.py`

```python
# Listen for events FROM the frontend
@socketio.on('solve')
def handle_solve(data):
    benchmark = data.get('benchmark')
    
    # Load the problem
    result = solver_manager.load_benchmark(benchmark)
    
    # Send event TO the frontend
    emit('loaded', {
        'cities': result['cities'],
        'numCities': result['numCities']
    })
    
    # Run solver (sends progress events via callbacks)
    result = solver_manager.solve(params)
    
    # Send completion event
    emit('complete', result)
```

## Why Two Servers?

| Server | Port | Purpose |
|--------|------|---------|
| Next.js (frontend) | 3000 | Serves the web pages, handles UI |
| Flask (backend) | 5000 | Runs the ACO algorithm, heavy computation |

**Separation of concerns:**
- Frontend = What the user sees
- Backend = What the computer calculates

This pattern is called **client-server architecture**.

## CORS: Why It Matters

Browsers block requests between different "origins" (domain:port combinations) for security. 

- `localhost:3000` (frontend) ≠ `localhost:5000` (backend)
- We must explicitly allow this in the backend:

```python
# backend/config.py
CORS_ORIGINS = ['http://localhost:3000', 'http://localhost:3001', 'http://localhost:3002']
```

Without this, the browser would refuse to connect to the backend.
