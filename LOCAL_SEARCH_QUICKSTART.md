# Local Search Quick Start Guide

Get better TSP solutions in 30 seconds.

## What Is It?

Local search improves ACO-generated tours by eliminating edge crossings and finding local optima.

**Result:** 2-5% better solutions, often near-optimal (0.03% on berlin52!)

## How to Use

### CLI (Recommended)

```bash
# Enable local search (best quality)
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search

# Faster variant (2-opt only)
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search --2opt-only

# Apply to all ant tours (slower, better pheromone info)
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search --ls-mode all
```

### Python

```python
import aco_solver

# Load and setup
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()
colony = aco_solver.AntColony(graph, numAnts=30)

# Enable local search
colony.setUseLocalSearch(True)  # Enable 2-opt+3-opt
colony.setUse3Opt(True)          # Optional: disable for 2-opt only
colony.setLocalSearchMode("best") # Apply to best tour (recommended)

# Solve
tour = colony.solve(100)
print(f"Best: {tour.getDistance():.2f}")
```

### Direct Usage (Python)

```python
# Apply to any tour
tour = colony.solve(100)
aco_solver.LocalSearch.improve(tour, graph, use3opt=True)
print(f"After local search: {tour.getDistance():.2f}")
```

## Performance

| Problem | Without LS | With LS | Improvement |
|---------|------------|---------|-------------|
| berlin52 | 7773.6 (+3.1%) | 7590.2 (+0.6%) | 2.4% |
| eil51 | 466.7 (+9.6%) | 447.8 (+5.1%) | 4.0% |
| st70 | 750.7 (+11.2%) | 710.4 (+5.3%) | 5.4% |

**Runtime overhead:** ~5-10% (negligible for quality gain)

## When to Use

✅ **Use local search when:**
- Solution quality matters
- ~5-10% slower is acceptable
- You want near-optimal results

❌ **Skip local search when:**
- Benchmarking pure ACO
- Extreme speed requirements
- Comparing with other implementations

## Modes Explained

### --local-search (default: 2-opt+3-opt, mode="best")
- **Best quality:** Uses both algorithms
- **Minimal overhead:** ~5-10% slower
- **Recommended** for production use

### --local-search --2opt-only
- **Fast variant:** Only 2-opt
- **Good quality:** 70-90% of benefit
- **Lower cost:** O(n²) vs O(n³)

### --local-search --ls-mode all
- **Apply to all ants:** Better pheromone trails
- **Slower:** ~2-3× runtime
- **Advanced:** Use when ACO struggles

## Quick Examples

### Best Result
```bash
./ant_colony_tsp berlin52.tsp --iterations 100 --ants 30 --local-search
# Result: 7544.37 (0.03% above optimal!)
```

### Fast & Good
```bash
./ant_colony_tsp eil51.tsp --iterations 100 --local-search --2opt-only
# Result: ~447 (5% above optimal, very fast)
```

### Maximum Quality
```bash
./ant_colony_tsp st70.tsp --iterations 200 --ants 40 --local-search --ls-mode all
# Result: Best possible, slower
```

## Algorithm Overview

### 2-opt
- Eliminates edge crossings
- O(n²) per iteration
- 2-5 iterations to converge
- **Impact:** 70-90% of total improvement

### 3-opt
- More complex reconnections
- O(n³) per iteration
- 1-3 iterations to converge
- **Impact:** Additional 10-30% improvement

## Tips

1. **Always use `--local-search`** unless benchmarking pure ACO
2. **Default settings** work well for most problems
3. **Use `--2opt-only`** if runtime is critical
4. **Combine with threading** for best performance:
   ```bash
   ./ant_colony_tsp berlin52.tsp --iterations 200 --local-search --threads 8
   ```

## More Information

- **Full documentation:** See `CLAUDE.md` (LocalSearch class section)
- **Benchmarks:** See `BENCHMARKS.md`
- **Implementation details:** See `LOCAL_SEARCH_SUMMARY.md`

## Quick Comparison

```bash
# Without local search
./ant_colony_tsp berlin52.tsp --iterations 100
# Result: ~7774 (+3.1% above optimal)

# With local search
./ant_colony_tsp berlin52.tsp --iterations 100 --local-search
# Result: ~7590 (+0.6% above optimal)

# Improvement: 184 distance units, 2.4% better!
```

---

**TL;DR:** Add `--local-search` to your commands for 2-5% better solutions with minimal overhead.
