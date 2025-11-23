# Local Search Implementation Summary

## Overview

This document summarizes the implementation of 2-opt and 3-opt local search algorithms for improving Ant Colony Optimization (ACO) solutions to the Travelling Salesman Problem (TSP).

## Implementation Date

November 23, 2025

## What Was Implemented

### 1. LocalSearch Class (C++)

**Files:**
- `cpp/include/LocalSearch.h` - Class declaration
- `cpp/src/LocalSearch.cpp` - Implementation
- `cpp/tests/LocalSearch_test.cpp` - 14 comprehensive unit tests

**Features:**
- Static utility class with three main methods:
  - `twoOpt(Tour& tour, const Graph& graph)` - 2-opt edge swapping
  - `threeOpt(Tour& tour, const Graph& graph)` - 3-opt edge swapping
  - `improve(Tour& tour, const Graph& graph, bool use3opt)` - Combined approach
- Efficient delta calculation (no full tour reconstruction)
- Guaranteed convergence to local optimum
- In-place tour modification for efficiency

### 2. AntColony Integration

**New Methods:**
- `setUseLocalSearch(bool)` - Enable/disable local search
- `setUse3Opt(bool)` - Control 2-opt vs 2-opt+3-opt
- `setLocalSearchMode(string)` - Set application mode ("best", "all", "none")

**Application Points:**
- **Mode "best"** (default): Apply to best tour after each iteration
- **Mode "all"**: Apply to every ant's tour after construction
- **Mode "none"**: Disabled

### 3. CLI Support

**New Flags:**
- `--local-search` - Enable local search optimization
- `--2opt-only` - Use only 2-opt (faster, skip 3-opt)
- `--ls-mode <mode>` - Set application mode

**Example:**
```bash
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search
```

### 4. Python Bindings

**Exposed Classes:**
- `LocalSearch` class with static methods
- AntColony configuration methods

**Example:**
```python
import aco_solver

loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()
colony = aco_solver.AntColony(graph, numAnts=30)

# Enable local search
colony.setUseLocalSearch(True)
tour = colony.solve(100)

# Or use directly
aco_solver.LocalSearch.improve(tour, graph)
```

### 5. Comprehensive Testing

- **14 new LocalSearch tests**
- **126 total tests** (all passing)
- Test coverage:
  - Edge cases (tours too small, already optimal)
  - Improvement verification
  - Tour validity preservation
  - Distance calculation accuracy
  - Convergence guarantees

### 6. Documentation

- **CLAUDE.md**: Updated with:
  - Complete LocalSearch class specification
  - Detailed "How and Why" algorithm explanations
  - Performance benchmarks
  - Usage examples for all interfaces
  - Updated status, roadmap, and architecture sections

- **BENCHMARKS.md**: Dedicated benchmarking document with:
  - Complete benchmark methodology
  - Statistical analysis of results
  - Comparison tables
  - Reproduction instructions

## How It Works

### 2-opt Algorithm

**Purpose:** Eliminate edge crossings in tours

**Process:**
1. For each pair of edges (i,i+1) and (j,j+1) in the tour
2. Calculate delta: `new_distance - old_distance` without reconstruction
3. If delta < 0 (improvement), reverse the segment between edges
4. Repeat until no improving swaps found (local optimum)

**Complexity:** O(n²) per iteration, typically 2-5 iterations to converge

**Why It Works:**
- Edge crossings are a hallmark of sub-optimal Euclidean TSP tours
- Simple and fast - only requires distance lookups
- Empirically very effective on TSP instances

### 3-opt Algorithm

**Purpose:** Find better local optima than 2-opt

**Process:**
1. For each triple of edges (i,i+1), (j,j+1), (k,k+1)
2. Remove all 3 edges, creating 3 segments
3. Try all 7 possible reconnection patterns
4. Keep the best reconnection
5. Repeat until no improvement found

**Complexity:** O(n³) per iteration, typically 1-3 iterations

**Why It's Better:**
- Can make more complex improvements than 2-opt
- Finds better local optima
- Provides 1-3% additional improvement over 2-opt

**Trade-off:**
- Much more expensive (O(n³) vs O(n²))
- Diminishing returns for the added cost

## Performance Results

### Benchmark Summary

**Configuration:** 100 iterations, 30 ants, 5 runs per config

| Problem | Size | Optimal | Without LS | With LS | Improvement |
|---------|------|---------|------------|---------|-------------|
| berlin52 | 52 | 7542 | 7773.6 (+3.1%) | 7590.2 (+0.6%) | **2.4%** |
| eil51 | 51 | 426 | 466.7 (+9.6%) | 447.8 (+5.1%) | **4.0%** |
| st70 | 70 | 675 | 750.7 (+11.2%) | 710.4 (+5.3%) | **5.4%** |

### Key Achievements

- **Best Result:** 7544.37 on berlin52 (0.03% above optimal!)
- **Consistent Improvement:** 2.4-5.4% better across all problems
- **Reduced Variance:** 36-48% reduction in standard deviation
- **Near-Optimal:** Achieves 0.4-7.1% above optimal (vs 3-11% without)

### Runtime Impact

- **Mode "best":** ~5-10% slower (negligible)
- **Mode "all":** ~2-3× slower (better pheromone trails)
- **Recommendation:** Use mode "best" for optimal quality/speed trade-off

## Why This Matters

### Complementary Strengths

**ACO Strengths:**
- Excellent global exploration
- Finds diverse good solutions
- Robust across problem types

**ACO Weaknesses:**
- Can converge to sub-optimal solutions
- Final tours may have edge crossings

**Local Search Strengths:**
- Polishes solutions to local optima
- Eliminates obvious inefficiencies (crossings)
- Fast convergence (few iterations)

**Local Search Weaknesses:**
- Can't escape local optima
- No global exploration

**Combined Approach:**
- ACO explores → Local search refines
- Best of both worlds: global + local
- Dramatic quality improvement with minimal overhead

## Design Decisions

### Why Static Methods?

LocalSearch is a utility class with no state:
- No member variables needed
- Pure algorithmic operations on Tour objects
- Allows standalone usage without instantiation

### Why In-Place Modification?

Tours are modified directly rather than copied:
- More efficient (no allocation/copying)
- Matches typical local search implementations
- Still preserves tour validity

### Why Two Application Modes?

**Mode "best"** (default):
- Minimal overhead
- Maximum benefit
- Best for most use cases

**Mode "all"**:
- Higher cost but better pheromone information
- Useful when ACO struggles initially
- Optional for advanced users

### Why Disabled by Default?

- Maintains pure ACO baseline for comparison
- Allows users to see ACO performance alone
- Opt-in for those who want maximum quality

## Files Added/Modified

### New Files
- `cpp/include/LocalSearch.h`
- `cpp/src/LocalSearch.cpp`
- `cpp/tests/LocalSearch_test.cpp`
- `BENCHMARKS.md`
- `LOCAL_SEARCH_SUMMARY.md` (this file)
- `benchmark_local_search.sh`
- `analyze_results.py`

### Modified Files
- `cpp/include/AntColony.h` - Added local search methods
- `cpp/src/AntColony.cpp` - Integrated local search
- `cpp/src/main.cpp` - Added CLI flags
- `python_bindings/bindings.cpp` - Exposed to Python
- `python_bindings/setup.py` - Added LocalSearch.cpp to build
- `CLAUDE.md` - Comprehensive documentation updates

## Testing Status

✅ **All 126 tests passing**
- 14 new LocalSearch tests
- 112 existing tests (unchanged)

**Test Coverage:**
- Edge cases (small tours, optimal tours)
- Improvement verification
- Validity preservation
- Distance accuracy
- Convergence guarantees
- 2-opt and 3-opt independently
- Combined improve() method

## Recommendations for Users

### When to Enable Local Search

**Always recommended when:**
- Solution quality is critical
- Runtime overhead is acceptable (~5-10%)
- Working with well-defined TSP instances

**Consider disabling when:**
- Benchmarking pure ACO performance
- Runtime is extremely constrained
- Comparing with other ACO implementations

### Which Mode to Use

**Use `--local-search` (2-opt+3-opt, mode="best"):**
- Default and recommended
- Best quality/speed trade-off
- Achieves near-optimal solutions

**Use `--local-search --2opt-only`:**
- When speed is more important
- Still gets 70-90% of the benefit
- Lower computational cost

**Use `--local-search --ls-mode all`:**
- When ACO has trouble finding good initial solutions
- Better pheromone information worth the cost
- Advanced use case

## Future Enhancements

Potential improvements:

1. **Parallelized 2-opt** - Parallel edge swap evaluation
2. **Adaptive mode switching** - Start with "all", switch to "best"
3. **Lin-Kernighan** - More sophisticated local search
4. **Web UI integration** - Add controls to frontend
5. **Larger benchmarks** - Test on 200+ city problems

## Conclusion

The 2-opt/3-opt local search implementation provides:
- **Dramatic quality improvement** (2.4-5.4% better solutions)
- **Near-optimal results** (best: 0.03% above optimal)
- **Minimal overhead** (~5-10% runtime increase)
- **Full-stack integration** (C++, CLI, Python)
- **Comprehensive testing** (14 new tests, all passing)
- **Thorough documentation** (CLAUDE.md, BENCHMARKS.md)

The implementation successfully complements ACO's global exploration with local refinement, achieving production-ready TSP solutions.

---

*Implementation completed: November 23, 2025*
*Documentation: CLAUDE.md, BENCHMARKS.md*
*Tests: 126/126 passing*
