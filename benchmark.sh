#!/bin/bash
# Benchmark script to compare serial vs parallel performance

echo "========================================="
echo "OpenMP Parallelization Benchmark"
echo "========================================="
echo ""

# Get number of CPU cores
NUM_CORES=$(nproc)
echo "System has $NUM_CORES CPU cores"
echo ""

# Test files
FILES=("ulysses16.tsp" "berlin52.tsp" "eil76.tsp" "kroA100.tsp")

for FILE in "${FILES[@]}"; do
    echo "========================================="
    echo "Testing: $FILE"
    echo "========================================="

    # Test with 1 thread (serial)
    echo "Running with 1 thread (serial)..."
    export OMP_NUM_THREADS=1
    START=$(date +%s.%N)
    ./bin/ant_colony_tsp "$FILE" --ants 30 --iterations 100 > /dev/null 2>&1
    END=$(date +%s.%N)
    TIME_1=$(echo "$END - $START" | bc)
    echo "Time with 1 thread: ${TIME_1}s"

    # Test with 2 threads
    echo "Running with 2 threads..."
    export OMP_NUM_THREADS=2
    START=$(date +%s.%N)
    ./bin/ant_colony_tsp "$FILE" --ants 30 --iterations 100 > /dev/null 2>&1
    END=$(date +%s.%N)
    TIME_2=$(echo "$END - $START" | bc)
    SPEEDUP_2=$(echo "scale=2; $TIME_1 / $TIME_2" | bc)
    echo "Time with 2 threads: ${TIME_2}s (${SPEEDUP_2}x speedup)"

    # Test with 4 threads
    echo "Running with 4 threads..."
    export OMP_NUM_THREADS=4
    START=$(date +%s.%N)
    ./bin/ant_colony_tsp "$FILE" --ants 30 --iterations 100 > /dev/null 2>&1
    END=$(date +%s.%N)
    TIME_4=$(echo "$END - $START" | bc)
    SPEEDUP_4=$(echo "scale=2; $TIME_1 / $TIME_4" | bc)
    echo "Time with 4 threads: ${TIME_4}s (${SPEEDUP_4}x speedup)"

    # Test with all cores
    if [ $NUM_CORES -gt 4 ]; then
        echo "Running with $NUM_CORES threads (all cores)..."
        export OMP_NUM_THREADS=$NUM_CORES
        START=$(date +%s.%N)
        ./bin/ant_colony_tsp "$FILE" --ants 30 --iterations 100 > /dev/null 2>&1
        END=$(date +%s.%N)
        TIME_N=$(echo "$END - $START" | bc)
        SPEEDUP_N=$(echo "scale=2; $TIME_1 / $TIME_N" | bc)
        echo "Time with $NUM_CORES threads: ${TIME_N}s (${SPEEDUP_N}x speedup)"
    fi

    echo ""
done

echo "========================================="
echo "Benchmark Complete!"
echo "========================================="
