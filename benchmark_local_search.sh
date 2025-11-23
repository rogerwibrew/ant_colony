#!/bin/bash
# Benchmark script to measure local search impact on solution quality
# Compares ACO with and without local search on multiple TSP instances

echo "========================================="
echo "Local Search Benchmark Suite"
echo "========================================="
echo ""

# Test problems with known optimal solutions
# Format: filename:optimal_distance:size
PROBLEMS=(
    "berlin52.tsp:7542:52"
    "eil51.tsp:426:51"
    "st70.tsp:675:70"
    "eil76.tsp:538:76"
    "kroA100.tsp:21282:100"
)

ITERATIONS=100
ANTS=30
RUNS=5  # Multiple runs for statistical significance

cd cpp/build/bin || exit 1

echo "Configuration:"
echo "  Iterations per run: $ITERATIONS"
echo "  Ants: $ANTS"
echo "  Runs per configuration: $RUNS"
echo ""

# Results arrays
declare -A results_without
declare -A results_with
declare -A results_2opt_only

for problem_data in "${PROBLEMS[@]}"; do
    IFS=':' read -r filename optimal size <<< "$problem_data"

    echo "========================================="
    echo "Problem: $filename (n=$size, optimal=$optimal)"
    echo "========================================="

    # Run without local search
    echo ""
    echo "Running WITHOUT local search ($RUNS runs)..."
    sum=0
    best=999999999
    worst=0
    for ((i=1; i<=$RUNS; i++)); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        echo "  Run $i: $result"
        sum=$(echo "$sum + $result" | bc)
        best=$(echo "if ($result < $best) $result else $best" | bc)
        worst=$(echo "if ($result > $worst) $result else $worst" | bc)
    done
    avg_without=$(echo "scale=2; $sum / $RUNS" | bc)
    gap_without=$(echo "scale=2; ($avg_without - $optimal) / $optimal * 100" | bc)
    results_without["$filename"]="$avg_without:$gap_without:$best:$worst"

    # Run WITH local search (2-opt + 3-opt)
    echo ""
    echo "Running WITH local search (2-opt + 3-opt, $RUNS runs)..."
    sum=0
    best=999999999
    worst=0
    for ((i=1; i<=$RUNS; i++)); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS --local-search 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        echo "  Run $i: $result"
        sum=$(echo "$sum + $result" | bc)
        best=$(echo "if ($result < $best) $result else $best" | bc)
        worst=$(echo "if ($result > $worst) $result else $worst" | bc)
    done
    avg_with=$(echo "scale=2; $sum / $RUNS" | bc)
    gap_with=$(echo "scale=2; ($avg_with - $optimal) / $optimal * 100" | bc)
    results_with["$filename"]="$avg_with:$gap_with:$best:$worst"

    # Run WITH 2-opt only
    echo ""
    echo "Running WITH 2-opt only ($RUNS runs)..."
    sum=0
    best=999999999
    worst=0
    for ((i=1; i<=$RUNS; i++)); do
        result=$(./ant_colony_tsp "$filename" --iterations $ITERATIONS --ants $ANTS --local-search --2opt-only 2>&1 | grep "Best tour distance:" | awk '{print $4}')
        echo "  Run $i: $result"
        sum=$(echo "$sum + $result" | bc)
        best=$(echo "if ($result < $best) $result else $best" | bc)
        worst=$(echo "if ($result > $worst) $result else $worst" | bc)
    done
    avg_2opt=$(echo "scale=2; $sum / $RUNS" | bc)
    gap_2opt=$(echo "scale=2; ($avg_2opt - $optimal) / $optimal * 100" | bc)
    results_2opt_only["$filename"]="$avg_2opt:$gap_2opt:$best:$worst"

    # Show improvement
    improvement=$(echo "scale=2; $avg_without - $avg_with" | bc)
    improvement_pct=$(echo "scale=2; $improvement / $avg_without * 100" | bc)

    echo ""
    echo "Summary for $filename:"
    echo "  Optimal:           $optimal"
    echo "  Without LS:        $avg_without (${gap_without}% above optimal)"
    echo "  With 2-opt only:   $avg_2opt (${gap_2opt}% above optimal)"
    echo "  With 2-opt+3-opt:  $avg_with (${gap_with}% above optimal)"
    echo "  Improvement:       $improvement ($improvement_pct%)"
    echo ""
done

# Final summary table
echo ""
echo "========================================="
echo "FINAL SUMMARY"
echo "========================================="
echo ""
printf "%-15s %8s %15s %15s %15s %12s\n" "Problem" "Optimal" "Without LS" "2-opt Only" "2-opt+3-opt" "Improvement"
printf "%-15s %8s %15s %15s %15s %12s\n" "-------" "-------" "----------" "----------" "-----------" "-----------"

for problem_data in "${PROBLEMS[@]}"; do
    IFS=':' read -r filename optimal size <<< "$problem_data"

    IFS=':' read -r avg_without gap_without best_without worst_without <<< "${results_without[$filename]}"
    IFS=':' read -r avg_2opt gap_2opt best_2opt worst_2opt <<< "${results_2opt_only[$filename]}"
    IFS=':' read -r avg_with gap_with best_with worst_with <<< "${results_with[$filename]}"

    improvement=$(echo "scale=1; $avg_without - $avg_with" | bc)
    improvement_pct=$(echo "scale=1; $improvement / $avg_without * 100" | bc)

    printf "%-15s %8s %15s %15s %15s %12s\n" \
        "$filename" \
        "$optimal" \
        "$avg_without (+${gap_without}%)" \
        "$avg_2opt (+${gap_2opt}%)" \
        "$avg_with (+${gap_with}%)" \
        "$improvement_pct%"
done

echo ""
echo "Note: All results averaged over $RUNS runs"
echo "Configuration: $ITERATIONS iterations, $ANTS ants per iteration"
echo ""
