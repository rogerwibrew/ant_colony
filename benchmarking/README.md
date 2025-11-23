# Benchmarking Suite - ACO TSP Solver

This directory contains a comprehensive benchmarking suite for measuring and comparing the performance of the ACO TSP solver across different execution modes (serial, multi-threaded CPU, and future GPU implementations).

## Purpose

The benchmark suite is designed to:

1. **Measure performance** across different hardware configurations
2. **Compare execution modes** (serial vs parallel)
3. **Validate speedup claims** (e.g., 10-12× improvement with OpenMP)
4. **Track performance regression** as code evolves
5. **Establish baseline metrics** for future GPU integration

## Quick Start

```bash
# Make the script executable
chmod +x run_benchmarks.sh

# Run the full benchmark suite
./run_benchmarks.sh

# Results will be saved to benchmarking/results/
```

## Benchmark Methodology

### Test Problems

The suite tests on 11 TSP problems of varying sizes:

| Category | Problem | Cities | Optimal Distance | Notes |
|----------|---------|--------|------------------|-------|
| Small | burma14.tsp | 14 | 3323 | Quick tests |
| Small | ulysses16.tsp | 16 | 6859 | Quick tests |
| Medium | berlin52.tsp | 52 | 7542 | Standard benchmark |
| Medium | eil76.tsp | 76 | 538 | Standard benchmark |
| Medium | kroA100.tsp | 100 | 21282 | Standard benchmark |
| Large | ch150.tsp | 150 | 6528 | Scalability test |
| Large | kroA200.tsp | 200 | 29368 | Scalability test |
| Extra Large | fl417.tsp | 417 | 11861 | High parallelism test |
| Extra Large | d493.tsp | 493 | 35002 | High parallelism test |
| Extra Extra Large | d1291.tsp | 1291 | 50801 | Maximum parallelism test |
| Extra Extra Large | fl1577.tsp | 1577 | 22249 | Maximum parallelism test |

### Thread Configurations

Each problem is tested with:

- **Serial (1 thread)**: Baseline single-threaded performance
- **4 threads**: Low parallelism
- **8 threads**: Medium parallelism
- **16 threads**: High parallelism
- **32 threads**: Maximum parallelism (typical server)
- **Auto-detect**: OpenMP default (usually matches core count)

### Trial Parameters

- **Trials per configuration**: 5 (for statistical validity)
- **Iterations per trial**: 100
- **Number of ants**: 30
- **Algorithm parameters**: α=1.0, β=2.0, ρ=0.5, Q=100.0

### Metrics Collected

For each test run:

1. **Execution time** (milliseconds) - Total wall-clock time
2. **Best tour distance** - Quality of solution found
3. **Gap from optimal** - Percentage above known optimal solution
4. **First iteration best** - Initial solution quality
5. **Overall improvement** - Improvement from first to final iteration

## Output Files

The benchmark script generates:

### 1. Raw Data CSV

`results/benchmark_YYYYMMDD_HHMMSS.csv`

Contains all raw measurements:
- Problem name
- Optimal distance
- Thread count
- Trial number
- Best distance found
- First iteration best
- Improvement percentage
- Execution time (ms)
- Gap from optimal (%)

**Example:**
```csv
Problem,Optimal,Threads,Trial,BestDistance,FirstIterationBest,Improvement,Time(ms),Gap(%)
berlin52.tsp,7542,1,1,8110.00,12733.58,4623.58,361,7.53
berlin52.tsp,7542,1,2,8032.50,13008.47,4975.96,368,6.50
```

### 2. Summary Report

`results/BENCHMARK_RESULTS_YYYYMMDD_HHMMSS.md`

Markdown report containing:
- System hardware information
- Test configuration
- Summary tables by problem size
- Speedup analysis
- Solution quality metrics
- Statistical analysis (mean, std dev)

## Analyzing Results

### Using the CSV Data

The raw CSV can be imported into:

- **Python/Pandas**: For custom analysis
  ```python
  import pandas as pd
  df = pd.read_csv('results/benchmark_YYYYMMDD_HHMMSS.csv')
  ```

- **Excel/Google Sheets**: For visualization
- **R**: For statistical analysis

### Key Metrics to Look For

#### 1. Speedup

```
Speedup = Serial Time / Parallel Time
```

Expected results:
- **4 threads**: 3-4× speedup
- **8 threads**: 5-7× speedup
- **16 threads**: 7-10× speedup
- **32 threads**: 10-12× speedup

#### 2. Efficiency

```
Efficiency = Speedup / Number of Threads
```

Expected behavior:
- Decreases with more threads (Amdahl's Law)
- Higher for larger problems
- 100% is perfect scaling (rarely achieved)

#### 3. Solution Quality

```
Gap = ((Best Distance - Optimal) / Optimal) × 100%
```

Expected results:
- Should be consistent across thread counts (algorithm is deterministic with fixed seed)
- Typically 1-5% above optimal for these problems

## Customizing Benchmarks

### Modify Test Problems

Edit the `PROBLEMS` array in `run_benchmarks.sh`:

```bash
declare -A PROBLEMS
PROBLEMS=(
    ["your_problem.tsp"]="optimal_distance"
)
```

### Modify Thread Counts

Edit the `THREAD_COUNTS` array:

```bash
THREAD_COUNTS=(1 2 4 8 16 32 64 0)  # 0 = auto
```

### Modify Trial Parameters

Change these variables:

```bash
NUM_TRIALS=10          # More trials = better statistics
ITERATIONS=200         # More iterations = better solutions
NUM_ANTS=50            # More ants = better exploration
```

## Comparing Results

### CPU vs GPU (Future)

When GPU implementation is ready:

1. Run CPU benchmarks: `./run_benchmarks.sh`
2. Run GPU benchmarks: `./run_gpu_benchmarks.sh` (future)
3. Compare the results:
   ```bash
   # Example comparison script
   python compare_benchmarks.py \
       results/benchmark_cpu_20251123.csv \
       results/benchmark_gpu_20251123.csv
   ```

### Before/After Code Changes

To detect performance regressions:

```bash
# Baseline
./run_benchmarks.sh
mv results/benchmark_*.csv results/baseline.csv

# After changes
./run_benchmarks.sh
mv results/benchmark_*.csv results/after_changes.csv

# Compare
diff <(tail -n +2 results/baseline.csv | sort) \
     <(tail -n +2 results/after_changes.csv | sort)
```

## Interpreting Variance

### Expected Variance Sources

1. **System load**: Other processes competing for CPU
2. **Thermal throttling**: CPU frequency changes
3. **Random number generation**: ACO is stochastic
4. **Cache effects**: Memory access patterns

### Reducing Variance

- Close all non-essential applications
- Run benchmarks during low system activity
- Increase `NUM_TRIALS` for more stable averages
- Disable CPU frequency scaling:
  ```bash
  # Linux: Set to performance mode
  sudo cpupower frequency-set -g performance
  ```

## Troubleshooting

### Binary Not Found

Error: `Binary not found at: cpp/build/bin/ant_colony_tsp`

Solution:
```bash
cd ../cpp/build
cmake --build .
```

### Out of Memory

For very large problems with many threads:

1. Reduce `NUM_ANTS`
2. Reduce `ITERATIONS`
3. Test fewer thread counts
4. Monitor with: `htop` or `free -h`

### Inconsistent Results

If results vary significantly between runs:

1. Check system load: `top` or `htop`
2. Check thermal throttling: `sensors` (if available)
3. Increase `NUM_TRIALS` to 10 or more
4. Run during off-hours with minimal background activity

## Benchmark Schedule

Recommended schedule for running benchmarks:

- **After major changes**: Algorithm modifications, optimization work
- **Before releases**: Version tagging, production deployment
- **Periodically**: Monthly, to track performance over time
- **Hardware changes**: New CPU, new compiler version, OS updates
- **Before/after GPU integration**: For comparison metrics

## Future Enhancements

Planned additions to the benchmark suite:

- [ ] GPU benchmark script (`run_gpu_benchmarks.sh`)
- [ ] Comparison visualization script (Python + matplotlib)
- [ ] Automated regression detection
- [ ] Performance tracking dashboard
- [ ] Memory usage profiling
- [ ] Energy consumption measurements

## References

- **Amdahl's Law**: Theoretical speedup limits
- **OpenMP Best Practices**: Thread scheduling, affinity
- **TSPLIB**: Standard TSP problem library
- **Statistical Analysis**: Confidence intervals, significance testing

---

**Last Updated**: 2025-11-23
**Benchmark Suite Version**: 1.0.0
