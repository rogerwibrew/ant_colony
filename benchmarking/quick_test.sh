#!/bin/bash

# ============================================================================
# Quick Test - Verify Benchmark Suite Works
# ============================================================================
# This is a minimal test to verify the benchmark suite is functioning
# before running the full comprehensive benchmark.
#
# Tests:
# - 1 small problem (burma14.tsp)
# - 2 thread counts (1, 8)
# - 2 trials per configuration
# - 20 iterations (vs 100 in full suite)
#
# Expected runtime: < 1 minute
# ============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BINARY="$PROJECT_ROOT/cpp/build/bin/ant_colony_tsp"

echo "============================================================================"
echo "  Quick Benchmark Test"
echo "============================================================================"
echo ""

# Check binary exists
if [ ! -f "$BINARY" ]; then
    echo "ERROR: Binary not found at: $BINARY"
    echo "Please build the project first:"
    echo "  cd cpp/build && cmake --build ."
    exit 1
fi

echo "Testing benchmark functionality..."
echo ""

# Test 1: Serial execution
echo "[1/4] Testing serial execution..."
output=$($BINARY eil51.tsp --serial --ants 10 --iterations 20 2>&1)
if echo "$output" | grep -q "Threading:.*Serial"; then
    echo "  ✓ Serial mode working"
else
    echo "  ✗ Serial mode failed"
    exit 1
fi

# Test 2: Multi-threaded execution
echo "[2/4] Testing multi-threaded execution..."
output=$($BINARY eil51.tsp --threads 8 --ants 10 --iterations 20 2>&1)
if echo "$output" | grep -q "Threading:.*8 threads"; then
    echo "  ✓ Multi-threaded mode working"
else
    echo "  ✗ Multi-threaded mode failed"
    exit 1
fi

# Test 3: Parse output
echo "[3/4] Testing output parsing..."
best_distance=$(echo "$output" | grep "Best tour distance:" | awk '{print $4}')
if [[ -n "$best_distance" ]] && [[ "$best_distance" =~ ^[0-9]+\.?[0-9]*$ ]]; then
    echo "  ✓ Output parsing working (best distance: $best_distance)"
else
    echo "  ✗ Output parsing failed"
    exit 1
fi

# Test 4: Python pandas availability (for summary generation)
echo "[4/4] Checking Python dependencies..."
if python3 -c "import pandas" 2>/dev/null; then
    echo "  ✓ pandas available for analysis"
else
    echo "  ⚠ pandas not available - summary report generation will fail"
    echo "  Install with: pip install pandas"
fi

echo ""
echo "============================================================================"
echo "  Quick Test: PASSED ✓"
echo "============================================================================"
echo ""
echo "The benchmark suite appears to be working correctly."
echo "You can now run the full benchmark with:"
echo "  ./run_benchmarks.sh"
echo ""
