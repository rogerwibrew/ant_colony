# Compatible TSPLIB Files (EUC_2D)

This directory contains **78 TSPLIB benchmark files** that are **fully compatible** with the ACO solver.

All files use **EUC_2D** (Euclidean 2D distance) edge weight type.

## Quick Start

```bash
cd build/bin

# Small problem (good for testing)
./ant_colony_tsp berlin52.tsp

# Medium problem
./ant_colony_tsp kroA100.tsp --ants 30 --iterations 200

# Large problem (be patient!)
./ant_colony_tsp d493.tsp --ants 50 --iterations 500
```

## File Categories

### Small (< 100 cities) - Perfect for quick tests
- berlin52.tsp (52) - Classic benchmark, optimal: 7542
- eil51.tsp, eil76.tsp, eil101.tsp
- st70.tsp (70)
- kroA100.tsp, kroB100.tsp, kroC100.tsp, kroD100.tsp, kroE100.tsp
- And more...

### Medium (100-500 cities) - Good balance
- kroA150.tsp, kroB150.tsp, kroA200.tsp, kroB200.tsp
- lin318.tsp, pcb442.tsp, rat195.tsp
- pr107, pr124, pr136, pr144, pr152, pr226, pr264, pr299, pr439
- And more...

### Large (500+ cities) - Serious challenge
- d493, d657, d1291, d1655, d2103, d15112, d18512
- fl417, fl1400, fl1577, fl3795
- pcb1173, pcb3038, pr1002, pr2392
- u574, u724, u1060, u1432, u1817, u2152, u2319
- vm1084, vm1748
- fnl4461, rl5915, rl5934, rl11849
- usa13509 (USA map!)

## Recommended Test Problems

### For Validation
- **berlin52** (optimal: 7542) - Should get ~7900 (5% above)
- **eil51** (optimal: 426) - Should get ~435 (2-3% above)
- **kroA100** (optimal: 21282) - Should get ~22000 (3-4% above)

## Usage Tips

```bash
# More ants for larger problems
./ant_colony_tsp d493.tsp --ants 100 --iterations 500

# Tune parameters
./ant_colony_tsp kroA100.tsp --alpha 1.5 --beta 3.0 --rho 0.3

# Quick test
./ant_colony_tsp berlin52.tsp --ants 10 --iterations 20
```

## Supported Format

**TSPLIB EUC_2D Format:**
```
NAME: problem_name
TYPE: TSP
DIMENSION: n
EDGE_WEIGHT_TYPE: EUC_2D
NODE_COORD_SECTION
1 x1 y1
2 x2 y2
...
EOF
```

**Simple Coordinate Format:**
```
n
0 x0 y0
1 x1 y1
...
```

## Incompatible Files

33 files (ATT, GEO, EXPLICIT, CEIL_2D types) have been moved to `../uncompatibleData/`.

See `../uncompatibleData/README.md` for details.

## References

- TSPLIB: http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/
- Optimal solutions: http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/STSP.html
