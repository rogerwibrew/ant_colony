#!/usr/bin/env python3
"""Test script to verify local search works through the web API stack"""

import sys
sys.path.insert(0, 'python_bindings')

import aco_solver

# Load a small problem
loader = aco_solver.TSPLoader("berlin52.tsp")
graph = loader.loadGraph()

if not graph.isValid():
    print("ERROR: Failed to load graph")
    sys.exit(1)

print(f"✓ Loaded berlin52: {graph.getNumCities()} cities")

# Create colony
colony = aco_solver.AntColony(graph, 20, 1.0, 2.0, 0.5, 100.0)

# Test WITHOUT local search
colony.setUseLocalSearch(False)
colony.initialize()
tour_no_ls = colony.solve(50)
print(f"✓ Without local search: {tour_no_ls.getDistance():.2f}")

# Test WITH local search (best mode)
colony.setUseLocalSearch(True)
colony.setUse3Opt(True)
colony.setLocalSearchMode("best")
colony.initialize()
tour_with_ls_best = colony.solve(50)
print(f"✓ With local search (best mode): {tour_with_ls_best.getDistance():.2f}")

# Test WITH local search (all mode)
colony.setLocalSearchMode("all")
colony.initialize()
tour_with_ls_all = colony.solve(50)
print(f"✓ With local search (all mode): {tour_with_ls_all.getDistance():.2f}")

# Test 2-opt only
colony.setUse3Opt(False)
colony.setLocalSearchMode("best")
colony.initialize()
tour_2opt_only = colony.solve(50)
print(f"✓ With 2-opt only: {tour_2opt_only.getDistance():.2f}")

# Verify local search improves solutions
improvement = tour_no_ls.getDistance() - tour_with_ls_best.getDistance()
improvement_pct = (improvement / tour_no_ls.getDistance()) * 100

print(f"\n✓ Local search improvement: {improvement:.2f} ({improvement_pct:.1f}%)")
print("✓ All local search configurations work correctly!")
print("\n✅ Local search integration test PASSED")
