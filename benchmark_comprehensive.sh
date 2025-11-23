#!/bin/bash
# Comprehensive benchmark with statistical analysis

echo "Comprehensive Local Search Benchmark"
echo "======================================"
echo ""

PROBLEMS=(
    "berlin52.tsp:7542:52"
    "eil51.tsp:426:51"
    "st70.tsp:675:70"
)

ITERATIONS=100
ANTS=30
RUNS=5

cd cpp/build/bin

for problem_data in "${PROBLEMS[@]}"; do
    IFS=':' read -r filename optimal size <<< "$problem_data"
    
    echo "Testing: $filename (n=$size, optimal=$optimal)"
    echo "================================================"
    
    # Arrays to store all results
    declare -a without_results
    declare -a with2opt_results
    declare -a with3opt_results
    
    echo ""
    echo "WITHOUT local search ($RUNS runs):"
    for i in $(seq 1 $RUNS); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        without_results+=($result)
        echo "  Run $i: $result"
    done
    
    echo ""
    echo "WITH 2-opt only ($RUNS runs):"
    for i in $(seq 1 $RUNS); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS --local-search --2opt-only 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        with2opt_results+=($result)
        echo "  Run $i: $result"
    done
    
    echo ""
    echo "WITH 2-opt + 3-opt ($RUNS runs):"
    for i in $(seq 1 $RUNS); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS --local-search 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        with3opt_results+=($result)
        echo "  Run $i: $result"
    done
    
    # Calculate statistics
    python3 << PYTHON
import sys
import statistics

def analyze(results, optimal, label):
    results = [float(x) for x in results]
    avg = statistics.mean(results)
    std = statistics.stdev(results) if len(results) > 1 else 0
    best = min(results)
    worst = max(results)
    gap_avg = ((avg - optimal) / optimal) * 100
    gap_best = ((best - optimal) / optimal) * 100
    gap_worst = ((worst - optimal) / optimal) * 100
    
    print(f"{label}:")
    print(f"  Average:  {avg:.2f} ({gap_avg:+.2f}% from optimal)")
    print(f"  Best:     {best:.2f} ({gap_best:+.2f}% from optimal)")
    print(f"  Worst:    {worst:.2f} ({gap_worst:+.2f}% from optimal)")
    print(f"  Std Dev:  {std:.2f}")
    return avg, best, gap_avg

optimal = $optimal
without = ${without_results[@]}
with2opt = ${with2opt_results[@]}
with3opt = ${with3opt_results[@]}

print()
print("STATISTICS:")
print("-" * 60)
avg_without, best_without, gap_without = analyze(without.split(), optimal, "Without Local Search")
print()
avg_2opt, best_2opt, gap_2opt = analyze(with2opt.split(), optimal, "With 2-opt Only")
print()
avg_3opt, best_3opt, gap_3opt = analyze(with3opt.split(), optimal, "With 2-opt + 3-opt")

print()
print("IMPROVEMENTS:")
print("-" * 60)
print(f"2-opt only vs baseline:      {((avg_without - avg_2opt) / avg_without * 100):.1f}% better")
print(f"2-opt+3-opt vs baseline:     {((avg_without - avg_3opt) / avg_without * 100):.1f}% better")
print(f"2-opt+3-opt vs 2-opt only:   {((avg_2opt - avg_3opt) / avg_2opt * 100):.1f}% better")
print()
PYTHON
    
    echo ""
    echo "================================================"
    echo ""
    
    # Clear arrays for next problem
    unset without_results
    unset with2opt_results
    unset with3opt_results
done
