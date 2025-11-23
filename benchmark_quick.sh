#!/bin/bash
# Quick benchmark for berlin52 only

echo "Quick Benchmark: berlin52 (3 runs each)"
echo "==========================================="
echo ""

PROBLEM="berlin52.tsp"
OPTIMAL=7542
ITERATIONS=100
ANTS=30
RUNS=3

cd cpp/build/bin

echo "WITHOUT local search:"
sum=0
for i in 1 2 3; do
    result=$(./ant_colony_tsp $PROBLEM --iterations $ITERATIONS --ants $ANTS 2>&1 | grep "Best tour distance:" | awk '{print $4}')
    echo "  Run $i: $result"
    sum=$(echo "$sum + $result" | bc)
done
avg_without=$(echo "scale=2; $sum / $RUNS" | bc)
gap_without=$(echo "scale=2; ($avg_without - $OPTIMAL) / $OPTIMAL * 100" | bc)
echo "  Average: $avg_without (${gap_without}% above optimal)"
echo ""

echo "WITH 2-opt only:"
sum=0
for i in 1 2 3; do
    result=$(./ant_colony_tsp $PROBLEM --iterations $ITERATIONS --ants $ANTS --local-search --2opt-only 2>&1 | grep "Best tour distance:" | awk '{print $4}')
    echo "  Run $i: $result"
    sum=$(echo "$sum + $result" | bc)
done
avg_2opt=$(echo "scale=2; $sum / $RUNS" | bc)
gap_2opt=$(echo "scale=2; ($avg_2opt - $OPTIMAL) / $OPTIMAL * 100" | bc)
echo "  Average: $avg_2opt (${gap_2opt}% above optimal)"
echo ""

echo "WITH 2-opt + 3-opt:"
sum=0
for i in 1 2 3; do
    result=$(./ant_colony_tsp $PROBLEM --iterations $ITERATIONS --ants $ANTS --local-search 2>&1 | grep "Best tour distance:" | awk '{print $4}')
    echo "  Run $i: $result"
    sum=$(echo "$sum + $result" | bc)
done
avg_with=$(echo "scale=2; $sum / $RUNS" | bc)
gap_with=$(echo "scale=2; ($avg_with - $OPTIMAL) / $OPTIMAL * 100" | bc)
echo "  Average: $avg_with (${gap_with}% above optimal)"
echo ""

improvement=$(echo "scale=2; (($avg_without - $avg_with) / $avg_without) * 100" | bc)
echo "Summary:"
echo "  Optimal:         $OPTIMAL"
echo "  Without LS:      $avg_without (+${gap_without}%)"
echo "  2-opt only:      $avg_2opt (+${gap_2opt}%)"
echo "  2-opt + 3-opt:   $avg_with (+${gap_with}%)"
echo "  Improvement:     ${improvement}%"
