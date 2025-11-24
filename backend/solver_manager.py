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

        num_cities = self.graph.getNumCities()

        # Extract parameters with defaults
        num_ants = params.get('numAnts')
        if num_ants is None:
            # Auto-calculate based on problem size (heuristic: 1-2 ants per city)
            num_ants = max(10, min(100, num_cities))

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

        # Local search parameters - apply smart defaults based on problem size
        use_local_search = params.get('useLocalSearch', False)
        use_3opt_param = params.get('use3Opt')
        local_search_mode_param = params.get('localSearchMode')

        # Smart defaults based on problem size
        # Small problems (<100 cities): can afford 3-opt + all mode
        # Medium problems (100-200 cities): use 3-opt + best mode
        # Large problems (>200 cities): use 2-opt only + best mode
        if use_local_search:
            if local_search_mode_param is None:
                # Auto-select mode based on problem size
                if num_cities < 100:
                    local_search_mode = 'best'  # Fast enough either way
                elif num_cities < 200:
                    local_search_mode = 'best'  # Only best tour for medium problems
                else:
                    local_search_mode = 'best'  # Only best tour for large problems
            else:
                local_search_mode = local_search_mode_param

            if use_3opt_param is None:
                # Auto-select 3-opt based on problem size and mode
                if local_search_mode == 'all':
                    # If user selected 'all' mode, disable 3-opt for problems >150 cities
                    use_3opt = num_cities < 150
                else:
                    # For 'best' mode, can use 3-opt up to ~300 cities
                    use_3opt = num_cities < 300
            else:
                use_3opt = use_3opt_param
        else:
            # Local search disabled
            use_3opt = False
            local_search_mode = 'none'

        # Warn about expensive configurations
        warnings = []
        if use_local_search and local_search_mode == 'all' and use_3opt and num_cities > 150:
            warnings.append(f"Warning: 'all tours' mode with 3-opt is very slow for {num_cities} cities. "
                          f"Consider using 'best' mode or disabling 3-opt for faster results.")

        if use_local_search and local_search_mode == 'all' and num_cities > 200:
            warnings.append(f"Warning: 'all tours' mode is computationally expensive for {num_cities} cities. "
                          f"Each iteration may take 5-10+ seconds. Consider using 'best' mode instead.")

        # Log configuration info
        print(f"Configuration for {num_cities} cities:")
        print(f"  - Ants: {num_ants}")
        print(f"  - Local search: {use_local_search}")
        if use_local_search:
            print(f"  - Mode: {local_search_mode}")
            print(f"  - 3-opt: {use_3opt}")
            if use_3opt_param is None or local_search_mode_param is None:
                print(f"  - (Auto-adjusted for problem size)")

        if warnings:
            for warning in warnings:
                print(warning)
                # Emit warning to frontend
                self.socketio.emit('warning', {'message': warning})

        # Elitist strategy parameters
        use_elitist = params.get('useElitist', False)
        elitist_weight = params.get('elitistWeight')  # None = use default (numAnts)
        pheromone_mode = params.get('pheromoneMode', 'all')
        rank_size = params.get('rankSize')  # None = use default (numAnts/2)

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

        # Configure local search
        colony.setUseLocalSearch(use_local_search)
        colony.setUse3Opt(use_3opt)
        colony.setLocalSearchMode(local_search_mode)

        # Configure elitist strategy
        colony.setUseElitist(use_elitist)
        if elitist_weight is not None:
            colony.setElitistWeight(elitist_weight)
        colony.setPheromoneMode(pheromone_mode)
        if rank_size is not None:
            colony.setRankSize(rank_size)

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

        # Calculate optimality gap if we know the optimal distance
        best_distance = best_tour.getDistance()
        optimal_distance = None
        optimality_gap = None

        if self.benchmark_name and self.benchmark_name in Config.BENCHMARKS:
            optimal_distance = Config.BENCHMARKS[self.benchmark_name]['optimal']
            # Calculate percentage above optimal: ((solution - optimal) / optimal) * 100
            optimality_gap = ((best_distance - optimal_distance) / optimal_distance) * 100

        return {
            'bestDistance': best_distance,
            'bestTour': best_tour.getSequence(),
            'convergenceHistory': global_bests,
            'cities': self.cities_coords,
            'elapsedTime': round(elapsed, 2),
            'totalIterations': total_iterations,
            'benchmark': self.benchmark_name,
            'optimalDistance': optimal_distance,
            'optimalityGap': round(optimality_gap, 2) if optimality_gap is not None else None
        }

    def stop(self):
        """Stop running solver"""
        self.is_running = False
