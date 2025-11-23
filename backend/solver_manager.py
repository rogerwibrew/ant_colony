"""Bridge between Flask and C++ ACO solver"""

import sys
import time
from pathlib import Path

# Add python_bindings to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'python_bindings'))

import aco_solver
from config import Config


class SolverManager:
    """Manages ACO solver execution with progress callbacks"""

    def __init__(self, socketio):
        self.socketio = socketio
        self.graph = None
        self.cities_coords = []
        self.benchmark_name = None
        self.start_time = None
        self.is_running = False

    def load_benchmark(self, benchmark_name):
        """Load TSPLIB benchmark file"""
        filepath = Config.DATA_DIR / benchmark_name

        if not filepath.exists():
            raise FileNotFoundError(f"Benchmark {benchmark_name} not found at {filepath}")

        # Load graph using TSPLoader (auto-searches data/ directories)
        loader = aco_solver.TSPLoader(str(filepath))
        self.graph = loader.loadGraph()

        if not self.graph.isValid():
            raise ValueError(f"Failed to load graph from {benchmark_name}")

        # Extract city coordinates for frontend visualization
        self.cities_coords = [
            [city.getX(), city.getY()]
            for city in self.graph.getCities()
        ]

        self.benchmark_name = benchmark_name

        return {
            'numCities': self.graph.getNumCities(),
            'cities': self.cities_coords,
            'benchmark': benchmark_name
        }

    def solve(self, params):
        """Run ACO solver with progress callbacks"""
        if self.graph is None or not self.graph.isValid():
            raise RuntimeError("No valid graph loaded")

        # Extract parameters with defaults
        num_ants = params.get('numAnts')
        if num_ants is None:
            # Auto-calculate based on problem size (heuristic: 1-2 ants per city)
            num_ants = max(10, min(100, self.graph.getNumCities()))

        iterations = params.get('iterations', Config.DEFAULT_PARAMS['iterations'])
        alpha = params.get('alpha', Config.DEFAULT_PARAMS['alpha'])
        beta = params.get('beta', Config.DEFAULT_PARAMS['beta'])
        rho = params.get('rho', Config.DEFAULT_PARAMS['rho'])
        Q = params.get('Q', Config.DEFAULT_PARAMS['Q'])

        # Convergence criterion
        use_convergence = params.get('useConvergence', False)
        convergence_iterations = params.get('convergenceIterations', 200)

        # Threading parameters
        use_parallel = params.get('useParallel', Config.DEFAULT_PARAMS['useParallel'])
        num_threads = params.get('numThreads', Config.DEFAULT_PARAMS['numThreads'])

        # Create colony
        colony = aco_solver.AntColony(
            self.graph,
            num_ants,
            alpha,
            beta,
            rho,
            Q
        )

        # Configure threading
        colony.setUseParallel(use_parallel)
        colony.setNumThreads(num_threads)

        # Initialize
        colony.initialize()

        # Set up progress callback
        self.start_time = time.time()
        self.is_running = True

        def progress_callback(iteration, best_distance, best_tour, convergence):
            """Called from C++ every N iterations"""
            if not self.is_running:
                return

            elapsed = time.time() - self.start_time

            # Calculate progress percentage
            if use_convergence:
                # For convergence mode, we don't know total iterations, so just show elapsed time
                progress_pct = 0  # Indeterminate progress
            else:
                progress_pct = (iteration / iterations) * 100

            # Convert iteration bests to running global bests for convergence graph
            global_bests = []
            running_best = float('inf')
            for dist in convergence:
                running_best = min(running_best, dist)
                global_bests.append(running_best)

            # Emit progress update via WebSocket
            self.socketio.emit('progress', {
                'iteration': iteration,
                'bestDistance': best_distance,
                'bestTour': best_tour,
                'convergenceHistory': global_bests,
                'cities': self.cities_coords,
                'elapsedTime': round(elapsed, 2),
                'progress': round(progress_pct, 1)
            })

        # Set callback interval (every 10 iterations by default)
        colony.setProgressCallback(progress_callback)
        colony.setCallbackInterval(10)

        # Configure convergence threshold if using convergence mode
        if use_convergence:
            colony.setConvergenceThreshold(convergence_iterations)

        # Solve (this releases GIL, allowing other Python threads to run)
        # Pass -1 for convergence mode, which tells C++ to run until no improvement
        max_iterations = -1 if use_convergence else iterations
        best_tour = colony.solve(max_iterations)

        # Send final result
        elapsed = time.time() - self.start_time
        self.is_running = False

        # Get convergence data (iteration bests from C++)
        iteration_bests = colony.getConvergenceData()
        total_iterations = len(iteration_bests)

        # Convert iteration bests to running global bests (cumulative minimum)
        # This ensures the convergence graph always shows non-increasing values
        global_bests = []
        running_best = float('inf')
        for dist in iteration_bests:
            running_best = min(running_best, dist)
            global_bests.append(running_best)

        return {
            'bestDistance': best_tour.getDistance(),
            'bestTour': best_tour.getSequence(),
            'convergenceHistory': global_bests,
            'cities': self.cities_coords,
            'elapsedTime': round(elapsed, 2),
            'totalIterations': total_iterations,
            'benchmark': self.benchmark_name
        }

    def stop(self):
        """Stop running solver"""
        self.is_running = False
