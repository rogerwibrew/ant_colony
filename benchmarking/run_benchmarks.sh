#!/bin/bash

# ============================================================================
# ACO TSP Solver - CPU Performance Benchmark Suite
# ============================================================================
#
# This script runs comprehensive performance benchmarks comparing:
# - Single-threaded vs multi-threaded execution
# - Various thread counts (1, 4, 8, 16, 32, auto)
# - Multiple TSP problem sizes (small, medium, large)
# - Multiple trials for statistical validity
#
# Results are saved to benchmarking/results/
# ============================================================================

set -e  # Exit on error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BINARY="$PROJECT_ROOT/cpp/build/bin/ant_colony_tsp"
RESULTS_DIR="$SCRIPT_DIR/results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RESULTS_FILE="$RESULTS_DIR/benchmark_${TIMESTAMP}.csv"
SUMMARY_FILE="$RESULTS_DIR/BENCHMARK_RESULTS_${TIMESTAMP}.md"

# Test parameters
NUM_TRIALS=5
ITERATIONS=100
NUM_ANTS=30

# Test problems (name, optimal_distance)
declare -A PROBLEMS
PROBLEMS=(
    ["eil51.tsp"]="426"
    ["pr76.tsp"]="108159"
    ["berlin52.tsp"]="7542"
    ["eil76.tsp"]="538"
    ["kroA100.tsp"]="21282"
    ["ch150.tsp"]="6528"
    ["kroA200.tsp"]="29368"
    ["fl417.tsp"]="11861"
    ["d493.tsp"]="35002"
    ["d1291.tsp"]="50801"
    ["fl1577.tsp"]="22249"
)

# Thread counts to test
THREAD_COUNTS=(1 4 8 16 32 0)  # 0 = auto-detect

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# ============================================================================
# Helper Functions
# ============================================================================

log() {
    echo -e "${BLUE}[$(date +%H:%M:%S)]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[$(date +%H:%M:%S)]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[$(date +%H:%M:%S)]${NC} $1"
}

# Extract metrics from CLI output
parse_output() {
    local output="$1"
    local best_distance=$(echo "$output" | grep "Best tour distance:" | awk '{print $4}')
    local first_best=$(echo "$output" | grep "First iteration best:" | awk '{print $4}')
    local overall_improvement=$(echo "$output" | grep "Improvement:" | awk '{print $2}')

    echo "${best_distance},${first_best},${overall_improvement}"
}

# ============================================================================
# System Information
# ============================================================================

get_system_info() {
    log "Gathering system information..."

    echo "# System Information" > "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"
    echo "**Date:** $(date '+%Y-%m-%d %H:%M:%S')" >> "$SUMMARY_FILE"
    echo "**Hostname:** $(hostname)" >> "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"

    echo "## Hardware" >> "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"
    echo "**CPU:**" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    lscpu | grep -E "Model name|CPU\(s\)|Thread|Core|Socket" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"

    echo "**Memory:**" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    free -h | grep -E "Mem|total" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"

    echo "**OpenMP:**" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    echo "Version: $(echo | gcc -fopenmp -dM -E - | grep _OPENMP)" >> "$SUMMARY_FILE"
    echo "\`\`\`" >> "$SUMMARY_FILE"
    echo "" >> "$SUMMARY_FILE"
}

# ============================================================================
# Main Benchmark Loop
# ============================================================================

run_benchmarks() {
    log "Starting benchmark suite..."
    log "Results will be saved to: $RESULTS_FILE"

    # Create CSV header
    echo "Problem,Optimal,Threads,Trial,BestDistance,FirstIterationBest,Improvement,Time(ms),Gap(%)" > "$RESULTS_FILE"

    local total_tests=$((${#PROBLEMS[@]} * ${#THREAD_COUNTS[@]} * NUM_TRIALS))
    local current_test=0

    # Iterate over problems
    for problem in "${!PROBLEMS[@]}"; do
        local optimal="${PROBLEMS[$problem]}"
        log "Testing problem: $problem (optimal: $optimal)"

        # Iterate over thread counts
        for threads in "${THREAD_COUNTS[@]}"; do
            local thread_label="$threads"
            if [ "$threads" -eq 0 ]; then
                thread_label="auto"
            fi

            log "  Thread count: $thread_label"

            # Run multiple trials
            for trial in $(seq 1 $NUM_TRIALS); do
                current_test=$((current_test + 1))
                local progress=$((current_test * 100 / total_tests))

                log "    Trial $trial/$NUM_TRIALS (${progress}% complete)"

                # Build command
                local cmd="$BINARY $problem --ants $NUM_ANTS --iterations $ITERATIONS"
                if [ "$threads" -eq 1 ]; then
                    cmd="$cmd --serial"
                elif [ "$threads" -gt 0 ]; then
                    cmd="$cmd --threads $threads"
                fi
                # threads=0 uses auto-detect (no flag needed)

                # Run benchmark and capture output + timing
                local start_time=$(date +%s%N)
                local output=$($cmd 2>&1)
                local end_time=$(date +%s%N)

                # Calculate elapsed time in milliseconds
                local elapsed_ms=$(( (end_time - start_time) / 1000000 ))

                # Parse results
                local metrics=$(parse_output "$output")
                IFS=',' read -r best_distance first_best improvement <<< "$metrics"

                # Calculate gap from optimal
                local gap=$(awk "BEGIN {printf \"%.2f\", (($best_distance - $optimal) / $optimal) * 100}")

                # Save to CSV
                echo "$problem,$optimal,$thread_label,$trial,$best_distance,$first_best,$improvement,$elapsed_ms,$gap" >> "$RESULTS_FILE"

                log_success "      Time: ${elapsed_ms}ms, Distance: $best_distance, Gap: ${gap}%"
            done
        done
        echo "" # Blank line between problems
    done

    log_success "Benchmark suite completed!"
}

# ============================================================================
# Generate Summary Report
# ============================================================================

generate_summary() {
    log "Generating summary report..."

    cat >> "$SUMMARY_FILE" << 'EOF'
---

# Benchmark Results - CPU Performance (OpenMP)

## Test Configuration

**Test Parameters:**
- Iterations: 100
- Number of Ants: 30
- Trials per configuration: 5
- Thread counts tested: 1, 4, 8, 16, 32, auto-detect

**Problems Tested:**

| Problem | Cities | Optimal Distance |
|---------|--------|------------------|
EOF

    # Add problem info
    for problem in eil51.tsp pr76.tsp berlin52.tsp eil76.tsp kroA100.tsp ch150.tsp kroA200.tsp fl417.tsp d493.tsp d1291.tsp fl1577.tsp; do
        if [[ -v PROBLEMS[$problem] ]]; then
            local cities=$(echo $problem | grep -oP '\d+')
            echo "| $problem | $cities | ${PROBLEMS[$problem]} |" >> "$SUMMARY_FILE"
        fi
    done

    cat >> "$SUMMARY_FILE" << 'EOF'

---

## Results Summary

Results are averaged across 5 trials. Time measurements in milliseconds.

### Small Problems (50-76 cities)

EOF

    # Generate summary tables by analyzing the CSV
    python3 << 'PYTHON_SCRIPT' >> "$SUMMARY_FILE"
import pandas as pd
import sys

# Read the CSV file
csv_file = sys.argv[1] if len(sys.argv) > 1 else None
if not csv_file:
    print("Error: No CSV file specified")
    sys.exit(1)

df = pd.read_csv(csv_file)

# Group by problem and threads, calculate statistics
summary = df.groupby(['Problem', 'Threads']).agg({
    'Time(ms)': ['mean', 'std'],
    'BestDistance': ['mean', 'std'],
    'Gap(%)': ['mean', 'std']
}).round(2)

# Problem categories
small_problems = ['eil51.tsp', 'pr76.tsp']
medium_problems = ['berlin52.tsp', 'eil76.tsp', 'kroA100.tsp']
large_problems = ['ch150.tsp', 'kroA200.tsp']
extra_large_problems = ['fl417.tsp', 'd493.tsp']
extra_extra_large_problems = ['d1291.tsp', 'fl1577.tsp']

def print_problem_table(problems, title):
    print(f"\n### {title}\n")

    for problem in problems:
        if problem not in summary.index.get_level_values(0):
            continue

        print(f"#### {problem}\n")
        print("| Threads | Avg Time (ms) | Std Dev | Speedup | Avg Distance | Gap from Optimal (%) |")
        print("|---------|---------------|---------|---------|--------------|----------------------|")

        problem_data = summary.loc[problem]
        serial_time = problem_data.loc[1, ('Time(ms)', 'mean')] if 1 in problem_data.index else None

        for threads in sorted(problem_data.index):
            avg_time = problem_data.loc[threads, ('Time(ms)', 'mean')]
            std_time = problem_data.loc[threads, ('Time(ms)', 'std')]
            avg_dist = problem_data.loc[threads, ('BestDistance', 'mean')]
            avg_gap = problem_data.loc[threads, ('Gap(%)', 'mean')]

            speedup = f"{serial_time / avg_time:.2f}×" if serial_time and avg_time > 0 else "1.00×"
            threads_label = "auto" if threads == 0 else str(threads)

            print(f"| {threads_label:^7} | {avg_time:>13.2f} | {std_time:>7.2f} | {speedup:>7} | {avg_dist:>12.2f} | {avg_gap:>20.2f} |")
        print()

# Print tables by category
print_problem_table(small_problems, "Small Problems (50-76 cities)")
print_problem_table(medium_problems, "Medium Problems (52-100 cities)")
print_problem_table(large_problems, "Large Problems (150-200 cities)")
print_problem_table(extra_large_problems, "Extra Large Problems (400-500 cities)")
print_problem_table(extra_extra_large_problems, "Extra Extra Large Problems (1000+ cities)")

# Overall statistics
print("\n---\n")
print("## Overall Performance Analysis\n")

# Calculate average speedup across all problems
speedups = []
for problem in df['Problem'].unique():
    problem_data = summary.loc[problem]
    if 1 in problem_data.index:
        serial_time = problem_data.loc[1, ('Time(ms)', 'mean')]
        for threads in problem_data.index:
            if threads != 1 and threads != 0:
                parallel_time = problem_data.loc[threads, ('Time(ms)', 'mean')]
                if parallel_time > 0:
                    speedups.append({
                        'problem': problem,
                        'threads': threads,
                        'speedup': serial_time / parallel_time
                    })

if speedups:
    speedup_df = pd.DataFrame(speedups)
    print("### Speedup by Thread Count (Average across all problems)\n")
    print("| Threads | Average Speedup | Min Speedup | Max Speedup |")
    print("|---------|-----------------|-------------|-------------|")

    for threads in sorted(speedup_df['threads'].unique()):
        thread_data = speedup_df[speedup_df['threads'] == threads]
        avg_speedup = thread_data['speedup'].mean()
        min_speedup = thread_data['speedup'].min()
        max_speedup = thread_data['speedup'].max()
        print(f"| {threads:^7} | {avg_speedup:>15.2f}× | {min_speedup:>11.2f}× | {max_speedup:>11.2f}× |")

print("\n### Solution Quality\n")
print("Average gap from optimal solution across all configurations:\n")

quality = df.groupby('Problem')['Gap(%)'].mean().round(2)
print("| Problem | Avg Gap from Optimal |")
print("|---------|----------------------|")
for problem, gap in quality.items():
    print(f"| {problem} | {gap:>19.2f}% |")

PYTHON_SCRIPT

    python3 -c "import sys; sys.argv.append('$RESULTS_FILE')" "$RESULTS_FILE" 2>/dev/null || echo "Python analysis failed - raw data available in CSV"

    cat >> "$SUMMARY_FILE" << EOF

---

## Raw Data

Complete benchmark data available in: \`$(basename $RESULTS_FILE)\`

To analyze further, import the CSV into your favorite data analysis tool.

---

## Notes

- All tests run on the same hardware configuration
- Each data point is the average of $NUM_TRIALS independent trials
- Times include full solver execution (initialization + $ITERATIONS iterations)
- OpenMP parallelization with dynamic scheduling
- Thread affinity not explicitly set (OS default)

EOF

    log_success "Summary report generated: $SUMMARY_FILE"
}

# ============================================================================
# Main Execution
# ============================================================================

main() {
    echo "============================================================================"
    echo "  ACO TSP Solver - CPU Performance Benchmark Suite"
    echo "============================================================================"
    echo ""

    # Check if binary exists
    if [ ! -f "$BINARY" ]; then
        log_warning "Binary not found at: $BINARY"
        log "Building project..."
        cd "$PROJECT_ROOT/cpp/build"
        cmake --build .
        cd "$SCRIPT_DIR"
    fi

    # Create results directory
    mkdir -p "$RESULTS_DIR"

    # Get system info
    get_system_info

    # Run benchmarks
    run_benchmarks

    # Generate summary
    generate_summary

    echo ""
    echo "============================================================================"
    echo "  Benchmark Complete!"
    echo "============================================================================"
    echo ""
    echo "Results saved to:"
    echo "  - CSV Data: $RESULTS_FILE"
    echo "  - Summary:  $SUMMARY_FILE"
    echo ""
}

# Run main function
main "$@"
