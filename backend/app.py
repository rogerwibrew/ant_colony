"""Flask API with WebSocket support for ACO TSP Solver"""

from flask import Flask, jsonify, request
from flask_socketio import SocketIO, emit
from flask_cors import CORS
import traceback

from config import Config
from solver_manager import SolverManager

# Initialize Flask app
app = Flask(__name__)
app.config.from_object(Config)

# Enable CORS
CORS(app, resources={r"/api/*": {"origins": Config.CORS_ORIGINS}})

# Initialize SocketIO
socketio = SocketIO(
    app,
    cors_allowed_origins=Config.SOCKETIO_CORS_ALLOWED_ORIGINS,
    async_mode=Config.SOCKETIO_ASYNC_MODE
)

# Global solver manager
solver_manager = SolverManager(socketio)

# ============================================================================
# REST Endpoints
# ============================================================================

@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'service': 'ACO TSP Solver API',
        'version': '1.0.0'
    })


@app.route('/api/benchmarks', methods=['GET'])
def list_benchmarks():
    """List available TSPLIB benchmark problems"""
    benchmarks = []

    for filename, metadata in Config.BENCHMARKS.items():
        filepath = Config.DATA_DIR / filename
        if filepath.exists():
            benchmarks.append({
                'name': filename,
                'cities': metadata['cities'],
                'optimal': metadata['optimal']
            })

    # Sort by number of cities
    benchmarks.sort(key=lambda x: x['cities'])

    return jsonify({
        'benchmarks': benchmarks,
        'count': len(benchmarks)
    })


@app.route('/api/benchmarks/<benchmark_name>', methods=['GET'])
def get_benchmark_info(benchmark_name):
    """Get information about a specific benchmark"""
    if benchmark_name not in Config.BENCHMARKS:
        return jsonify({'error': 'Benchmark not found'}), 404

    filepath = Config.DATA_DIR / benchmark_name
    if not filepath.exists():
        return jsonify({'error': 'Benchmark file not found on disk'}), 404

    metadata = Config.BENCHMARKS[benchmark_name]
    return jsonify({
        'name': benchmark_name,
        'cities': metadata['cities'],
        'optimal': metadata['optimal'],
        'filepath': str(filepath)
    })


@app.route('/api/parameters', methods=['GET'])
def get_default_parameters():
    """Get default ACO parameters"""
    return jsonify({
        'parameters': Config.DEFAULT_PARAMS,
        'descriptions': {
            'numAnts': 'Number of ants in the colony',
            'iterations': 'Number of optimization iterations',
            'alpha': 'Pheromone importance factor (higher = more pheromone influence)',
            'beta': 'Heuristic importance factor (higher = more distance influence)',
            'rho': 'Evaporation rate (0-1, higher = faster evaporation)',
            'Q': 'Pheromone deposit factor'
        }
    })


# ============================================================================
# WebSocket Events
# ============================================================================

@socketio.on('connect')
def handle_connect():
    """Client connected"""
    print('Client connected')
    emit('connected', {
        'message': 'Connected to ACO TSP Solver',
        'version': '1.0.0'
    })


@socketio.on('disconnect')
def handle_disconnect():
    """Client disconnected"""
    print('Client disconnected')
    solver_manager.stop()


@socketio.on('solve')
def handle_solve(data):
    """Start solving TSP problem"""
    try:
        # Extract request data
        benchmark = data.get('benchmark')
        params = data.get('params', {})

        if not benchmark:
            emit('error', {'message': 'No benchmark specified'})
            return

        print(f"Received solve request for: {benchmark}")
        print(f"Parameters: {params}")

        # Load benchmark
        load_result = solver_manager.load_benchmark(benchmark)

        # Send initial data
        emit('loaded', {
            'benchmark': benchmark,
            'numCities': load_result['numCities'],
            'cities': load_result['cities']
        })

        print(f"Loaded {benchmark}: {load_result['numCities']} cities")

        # Run solver (progress updates sent via callbacks)
        result = solver_manager.solve(params)

        print(f"Optimization complete: {result['bestDistance']:.2f} in {result['elapsedTime']:.2f}s")

        # Send final result
        emit('complete', result)

    except FileNotFoundError as e:
        print(f"File not found: {e}")
        emit('error', {'message': f'Benchmark not found: {str(e)}'})

    except ValueError as e:
        print(f"Value error: {e}")
        emit('error', {'message': f'Invalid data: {str(e)}'})

    except Exception as e:
        print(f"Error in solve: {traceback.format_exc()}")
        emit('error', {'message': f'Server error: {str(e)}'})


@socketio.on('preview')
def handle_preview(data):
    """Preview a benchmark (load cities without solving)"""
    try:
        benchmark = data.get('benchmark')

        if not benchmark:
            emit('error', {'message': 'No benchmark specified'})
            return

        print(f"Received preview request for: {benchmark}")

        # Load benchmark
        load_result = solver_manager.load_benchmark(benchmark)

        # Send cities data
        emit('preview_loaded', {
            'benchmark': benchmark,
            'numCities': load_result['numCities'],
            'cities': load_result['cities']
        })

        print(f"Preview loaded: {benchmark} with {load_result['numCities']} cities")

    except FileNotFoundError as e:
        print(f"File not found: {e}")
        emit('error', {'message': f'Benchmark not found: {str(e)}'})

    except Exception as e:
        print(f"Error in preview: {traceback.format_exc()}")
        emit('error', {'message': f'Server error: {str(e)}'})


@socketio.on('cancel')
def handle_cancel():
    """Cancel running optimization"""
    solver_manager.stop()
    emit('cancelled', {'message': 'Optimization cancelled'})
    print('Optimization cancelled by client')


# ============================================================================
# Error Handlers
# ============================================================================

@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return jsonify({'error': 'Not found'}), 404


@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    return jsonify({'error': 'Internal server error'}), 500


# ============================================================================
# Run Server
# ============================================================================

if __name__ == '__main__':
    print("=" * 60)
    print("ACO TSP Solver - Flask Backend with WebSocket Support")
    print("=" * 60)
    print(f"CORS origins: {Config.CORS_ORIGINS}")
    print(f"Data directory: {Config.DATA_DIR}")
    print(f"Debug mode: {Config.DEBUG}")
    print(f"Async mode: {Config.SOCKETIO_ASYNC_MODE}")
    print("=" * 60)
    print("REST Endpoints:")
    print("  GET  /api/health")
    print("  GET  /api/benchmarks")
    print("  GET  /api/benchmarks/<name>")
    print("  GET  /api/parameters")
    print("\nWebSocket Events:")
    print("  connect -> connected")
    print("  preview -> preview_loaded (load cities without solving)")
    print("  solve -> loaded, progress (every 10 iter), complete")
    print("  cancel -> cancelled")
    print("  disconnect")
    print("=" * 60)
    print(f"\nServer starting on http://0.0.0.0:5000")
    print("Press Ctrl+C to stop\n")

    socketio.run(
        app,
        host='0.0.0.0',
        port=5000,
        debug=Config.DEBUG,
        allow_unsafe_werkzeug=True  # For development only
    )
