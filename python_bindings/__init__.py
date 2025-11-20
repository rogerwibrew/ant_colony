"""
ACO Solver Python Bindings

High-performance Ant Colony Optimization solver for the Travelling Salesman Problem.

Example usage:
    >>> import aco_solver
    >>>
    >>> # Load TSPLIB benchmark
    >>> loader = aco_solver.TSPLoader("../data/berlin52.tsp")
    >>> graph = loader.loadGraph()
    >>>
    >>> # Create colony
    >>> colony = aco_solver.AntColony(graph, numAnts=20, alpha=1.0, beta=2.0)
    >>>
    >>> # Define progress callback
    >>> def progress(iteration, best_dist, best_tour, convergence):
    ...     print(f"Iteration {iteration}: {best_dist:.2f}")
    >>>
    >>> colony.setProgressCallback(progress)
    >>> colony.setCallbackInterval(10)
    >>>
    >>> # Solve
    >>> best_tour = colony.solve(100)
    >>> print(f"Best distance: {best_tour.getDistance():.2f}")

Classes:
    City: Represents a city with coordinates
    Graph: TSP problem instance with precomputed distances
    Tour: Solution representation
    TSPLoader: Load TSP files (coordinates, matrix, or TSPLIB format)
    PheromoneMatrix: Pheromone level tracking
    Ant: Individual ant agent
    AntColony: Main optimization algorithm
"""

from .aco_solver import (
    City,
    Graph,
    Tour,
    TSPLoader,
    PheromoneMatrix,
    Ant,
    AntColony
)

__version__ = "1.0.0"
__all__ = [
    'City',
    'Graph',
    'Tour',
    'TSPLoader',
    'PheromoneMatrix',
    'Ant',
    'AntColony'
]
