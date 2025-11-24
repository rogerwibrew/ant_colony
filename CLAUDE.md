# CLAUDE.md

Quick reference for Claude Code (claude.ai/code) when working with this repository.

## Project Overview

C++17 Ant Colony Optimization (ACO) implementation for Travelling Salesman Problem.

**Status:** ✅ 137 tests passing. Production-ready ACO solver with CLI, Python bindings, and web UI.

**Key Features:**
- OpenMP multi-threading (10-12× speedup on multi-core CPUs)
- 2-opt/3-opt local search (achieves 0.03% above optimal on berlin52)
- Elitist pheromone strategies (4 modes: all/best-iteration/best-so-far/rank)
- TSPLIB format support (113+ benchmark problems)

## Essential Commands

**Build & Test:**
```bash
# Build C++ core
cd cpp/build && cmake --build .

# Run tests
cd cpp/build && ctest --output-on-failure

# Run solver
./cpp/build/bin/ant_colony_tsp berlin52.tsp --local-search --threads 8
```

**Web Interface:**
```bash
make start  # Start frontend (3000) and backend (5000)
make stop   # Stop servers
```

**Python Bindings:**
```bash
cd python_bindings && python setup.py build_ext --inplace
python test_bindings.py
```

## Architecture (5 Core Classes)

1. **Graph** - Precomputed distance matrix (O(1) lookups)
2. **PheromoneMatrix** - Tracks pheromones with evaporation
3. **Ant** - Constructs tours probabilistically (main bottleneck ~60-70%)
4. **AntColony** - Algorithm coordinator (OpenMP parallel)
5. **LocalSearch** - 2-opt/3-opt improvement (5-15% solution quality gain)

**Execution Flow:**
```
Load TSP → Initialize Colony → For each iteration:
  → Ants build tours
  → [Optional] Local search
  → Update pheromones (evaporation + deposit)
  → Track best solution
→ Output best tour + convergence data
```

## Key Design Points

- **Distance matrix:** Precomputed once (O(n²)), O(1) lookups
- **Initial pheromone:** τ₀ = m / C^nn (m=ants, C^nn=nearest neighbor length)
- **Member variables:** Trailing underscore (`id_`, `cities_`)
- **Single-arg constructors:** `explicit`
- **OpenMP:** Parallel ant construction, thread-safe pheromone updates with atomics
- **Local search modes:** "best" (default, apply to best tour), "all" (all ant tours), "none"
- **Pheromone modes:** "all" (default), "best-iteration", "best-so-far", "rank"
- **Elitist strategy:** Best-so-far tour gets additional weighted pheromone deposit

## File Organization

```
cpp/              - C++ implementation (include/, src/, tests/)
data/             - TSPLIB benchmarks (113+ files, auto-discovered)
python_bindings/  - pybind11 bindings
backend/          - Flask API with WebSocket
frontend/         - Next.js web UI
docs/             - Detailed documentation (see below)
```

## Detailed Documentation

**See `docs/` for comprehensive information:**
- **quick-start.md** - Build, test, run commands
- **architecture.md** - Detailed class specifications
- **algorithms.md** - ACO algorithm details, parameters, local search
- **development.md** - Testing, profiling, roadmap, references
- **BENCHMARKS.md** - Performance benchmarks and methodology (in root)

## Current Focus

🚧 **UI improvements** - Preview feature, convergence stopping, ant controls

## Adding New Classes

1. Create `cpp/include/ClassName.h` and `cpp/src/ClassName.cpp`
2. Add `cpp/tests/ClassName_test.cpp`
3. Rebuild (CMake auto-discovers via GLOB_RECURSE)

## Quick Tips

- TSPLoader auto-searches: `data/`, `../data/`, `../../data/`, `../../../data/`
- Only EUC_2D edge weight type supported (ATT, GEO, EXPLICIT not yet implemented)
- Enable local search for best solution quality: `--local-search`
- Use multi-threading for large problems: `--threads N`
- Elitist strategy for faster convergence: `--elitist --pheromone-mode best-so-far`

## Performance Notes

- **Bottleneck:** `Ant::selectNextCity()` (~60-70% runtime)
- **OpenMP speedup:** 10-12× on 32 cores
- **Local search overhead:** ~5-10% runtime for 5-15% quality improvement
- **Solution quality:** 0.03%-1% above optimal on benchmarks with local search

---

**For detailed information on classes, algorithms, testing, or development, see the documentation in `docs/`.**
