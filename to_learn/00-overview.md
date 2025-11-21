# Learning Guide: ACO TSP Web Application

This folder contains explanations of the key technologies used in this project. Read these files to understand how everything works together.

## Files

1. **[01-frontend-backend-interaction.md](01-frontend-backend-interaction.md)** - How the Next.js frontend talks to the Flask backend
2. **[02-websockets-vs-rest.md](02-websockets-vs-rest.md)** - Why we use WebSockets instead of axios/REST for real-time updates
3. **[03-typescript-basics.md](03-typescript-basics.md)** - Understanding the TypeScript files in the frontend
4. **[04-pybind11-cpp-python.md](04-pybind11-cpp-python.md)** - How we connected C++ code to Python

## The Big Picture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           YOUR BROWSER                                   │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │              Next.js Frontend (TypeScript/React)                 │   │
│  │              http://localhost:3000                               │   │
│  └─────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ WebSocket Connection
                                    │ (real-time, two-way)
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        BACKEND SERVER                                    │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │              Flask + SocketIO (Python)                           │   │
│  │              http://localhost:5000                               │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                    │                                     │
│                                    │ Python function calls               │
│                                    │ (via pybind11)                      │
│                                    ▼                                     │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │              C++ ACO Solver (compiled to .so file)               │   │
│  │              aco_solver.cpython-*.so                             │   │
│  └─────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
```

## Key Files to Study

| Component | Key Files | Purpose |
|-----------|-----------|---------|
| Frontend | `frontend/src/hooks/useSocket.ts` | WebSocket connection logic |
| Frontend | `frontend/src/components/aco-tsp-dashboard.tsx` | Main UI component |
| Backend | `backend/app.py` | Flask server + WebSocket handlers |
| Backend | `backend/solver_manager.py` | Bridges Python ↔ C++ |
| Bindings | `python_bindings/bindings.cpp` | C++ ↔ Python bridge definitions |
