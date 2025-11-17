# Incompatible TSPLIB Files

This directory contains TSPLIB benchmark files that are **not currently supported** by the ACO solver.

The solver currently only supports **EUC_2D** (Euclidean 2D distance) edge weight types.

## Directory Structure

### ATT/ (2 files)
**Pseudo-Euclidean distance** - Used for certain geographical problems.

Files:
- att48.tsp - 48 capitals of the US
- att532.tsp - 532 cities

Formula: `d = ceil(sqrt((x1-x2)² + (y1-y2)²) / 10)`

### GEO/ (10 files)
**Geographical coordinates** - Latitude/longitude on Earth's surface.

Files:
- ali535.tsp, burma14.tsp, gr137.tsp, gr202.tsp, gr229.tsp
- gr431.tsp, gr666.tsp, gr96.tsp, ulysses16.tsp, ulysses22.tsp

Formula: Uses spherical distance (great circle distance)

### EXPLICIT/ (17 files)
**Pre-computed distance matrix** - Distances explicitly provided, not derived from coordinates.

Files:
- bayg29.tsp, bays29.tsp, brazil58.tsp, brg180.tsp, dantzig42.tsp
- fri26.tsp, gr120.tsp, gr17.tsp, gr21.tsp, gr24.tsp
- gr48.tsp, hk48.tsp, pa561.tsp, si1032.tsp, si175.tsp
- si535.tsp, swiss42.tsp

Note: Some files may have NODE_COORD_SECTION but use EXPLICIT distances.

### OTHER/ (4 files - CEIL_2D)
**Ceiling of Euclidean 2D** - Euclidean distance rounded up to nearest integer.

Files:
- dsj1000.tsp - 1000 cities
- pla7397.tsp - 7397 cities
- pla33810.tsp - 33810 cities
- pla85900.tsp - 85900 cities (largest!)

Formula: `d = ceil(sqrt((x1-x2)² + (y1-y2)²))`

## Implementation Status

❌ **Not Implemented** - These file types require different distance calculation methods.

## Future Implementation

To support these files, the following would need to be added:

1. **ATT Distance** - Modify `City::distanceTo()` or add distance function parameter
2. **GEO Distance** - Convert lat/long to radians, use haversine formula
3. **EXPLICIT** - Store distance matrix directly, skip coordinate-based calculation
4. **CEIL_2D** - Simple modification to round up Euclidean distances

## Total Count

- **Incompatible files**: 33
- **Compatible (EUC_2D) files in data/**: 78

## See Also

For compatible files and solver usage, see `data/README.md` and main `CLAUDE.md`.
