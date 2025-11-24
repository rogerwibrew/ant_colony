"""Configuration for Flask API and ACO Solver"""

import os
from pathlib import Path

class Config:
    """Flask + ACO configuration"""

    # Flask settings
    SECRET_KEY = os.environ.get('SECRET_KEY') or 'dev-secret-key-change-in-production'
    DEBUG = os.environ.get('FLASK_DEBUG', 'True').lower() == 'true'

    # CORS settings
    CORS_ORIGINS = os.environ.get('CORS_ORIGINS', 'http://localhost:3000,http://localhost:3001,http://localhost:3002').split(',')

    # WebSocket settings
    SOCKETIO_CORS_ALLOWED_ORIGINS = CORS_ORIGINS
    SOCKETIO_ASYNC_MODE = 'threading'  # Using threading instead of eventlet for Python 3.13 compatibility

    # ACO solver paths
    BASE_DIR = Path(__file__).parent.parent
    DATA_DIR = BASE_DIR / 'data'

    # Default ACO parameters
    DEFAULT_PARAMS = {
        'numAnts': 20,
        'iterations': 100,
        'alpha': 1.0,
        'beta': 2.0,
        'rho': 0.5,
        'Q': 100.0,
        'useParallel': True,      # Enable multi-threading by default
        'numThreads': 0,          # 0 = auto-detect cores
        'useLocalSearch': False,  # Disable local search by default
        'use3Opt': True,          # Use both 2-opt and 3-opt when local search enabled
        'localSearchMode': 'best' # Apply to best tour only ('best', 'all', or 'none')
    }

    # Benchmark metadata (name, optimal distance, number of cities)
    # Only including EUC_2D benchmarks that are in the data/ directory
    # All 78 TSPLIB benchmark problems available in data/
    BENCHMARKS = {
        'a280.tsp': {'cities': 280, 'optimal': 2579},
        'berlin52.tsp': {'cities': 52, 'optimal': 7542},
        'bier127.tsp': {'cities': 127, 'optimal': 118282},
        'brd14051.tsp': {'cities': 14051, 'optimal': 469385},
        'ch130.tsp': {'cities': 130, 'optimal': 6110},
        'ch150.tsp': {'cities': 150, 'optimal': 6528},
        'd198.tsp': {'cities': 198, 'optimal': 15780},
        'd493.tsp': {'cities': 493, 'optimal': 35002},
        'd657.tsp': {'cities': 657, 'optimal': 48912},
        'd1291.tsp': {'cities': 1291, 'optimal': 50801},
        'd1655.tsp': {'cities': 1655, 'optimal': 62128},
        'd2103.tsp': {'cities': 2103, 'optimal': 80450},
        'd15112.tsp': {'cities': 15112, 'optimal': 1573084},
        'd18512.tsp': {'cities': 18512, 'optimal': 645238},
        'eil51.tsp': {'cities': 51, 'optimal': 426},
        'eil76.tsp': {'cities': 76, 'optimal': 538},
        'eil101.tsp': {'cities': 101, 'optimal': 629},
        'fl417.tsp': {'cities': 417, 'optimal': 11861},
        'fl1400.tsp': {'cities': 1400, 'optimal': 20127},
        'fl1577.tsp': {'cities': 1577, 'optimal': 22249},
        'fl3795.tsp': {'cities': 3795, 'optimal': 28772},
        'fnl4461.tsp': {'cities': 4461, 'optimal': 182566},
        'gil262.tsp': {'cities': 262, 'optimal': 2378},
        'kroA100.tsp': {'cities': 100, 'optimal': 21282},
        'kroB100.tsp': {'cities': 100, 'optimal': 22141},
        'kroC100.tsp': {'cities': 100, 'optimal': 20749},
        'kroD100.tsp': {'cities': 100, 'optimal': 21294},
        'kroE100.tsp': {'cities': 100, 'optimal': 22068},
        'kroA150.tsp': {'cities': 150, 'optimal': 26524},
        'kroB150.tsp': {'cities': 150, 'optimal': 26130},
        'kroA200.tsp': {'cities': 200, 'optimal': 29368},
        'kroB200.tsp': {'cities': 200, 'optimal': 29437},
        'lin105.tsp': {'cities': 105, 'optimal': 14379},
        'lin318.tsp': {'cities': 318, 'optimal': 42029},
        'linhp318.tsp': {'cities': 318, 'optimal': 41345},
        'nrw1379.tsp': {'cities': 1379, 'optimal': 56638},
        'p654.tsp': {'cities': 654, 'optimal': 34643},
        'pcb442.tsp': {'cities': 442, 'optimal': 50778},
        'pcb1173.tsp': {'cities': 1173, 'optimal': 56892},
        'pcb3038.tsp': {'cities': 3038, 'optimal': 137694},
        'pr76.tsp': {'cities': 76, 'optimal': 108159},
        'pr107.tsp': {'cities': 107, 'optimal': 44303},
        'pr124.tsp': {'cities': 124, 'optimal': 59030},
        'pr136.tsp': {'cities': 136, 'optimal': 96772},
        'pr144.tsp': {'cities': 144, 'optimal': 58537},
        'pr152.tsp': {'cities': 152, 'optimal': 73682},
        'pr226.tsp': {'cities': 226, 'optimal': 80369},
        'pr264.tsp': {'cities': 264, 'optimal': 49135},
        'pr299.tsp': {'cities': 299, 'optimal': 48191},
        'pr439.tsp': {'cities': 439, 'optimal': 107217},
        'pr1002.tsp': {'cities': 1002, 'optimal': 259045},
        'pr2392.tsp': {'cities': 2392, 'optimal': 378032},
        'rat99.tsp': {'cities': 99, 'optimal': 1211},
        'rat195.tsp': {'cities': 195, 'optimal': 2323},
        'rat575.tsp': {'cities': 575, 'optimal': 6773},
        'rat783.tsp': {'cities': 783, 'optimal': 8806},
        'rd100.tsp': {'cities': 100, 'optimal': 7910},
        'rd400.tsp': {'cities': 400, 'optimal': 15281},
        'rl1304.tsp': {'cities': 1304, 'optimal': 252948},
        'rl1323.tsp': {'cities': 1323, 'optimal': 270199},
        'rl1889.tsp': {'cities': 1889, 'optimal': 316536},
        'rl5915.tsp': {'cities': 5915, 'optimal': 565530},
        'rl5934.tsp': {'cities': 5934, 'optimal': 556045},
        'rl11849.tsp': {'cities': 11849, 'optimal': 923288},
        'st70.tsp': {'cities': 70, 'optimal': 675},
        'ts225.tsp': {'cities': 225, 'optimal': 126643},
        'tsp225.tsp': {'cities': 225, 'optimal': 3916},
        'u159.tsp': {'cities': 159, 'optimal': 42080},
        'u574.tsp': {'cities': 574, 'optimal': 36905},
        'u724.tsp': {'cities': 724, 'optimal': 41910},
        'u1060.tsp': {'cities': 1060, 'optimal': 224094},
        'u1432.tsp': {'cities': 1432, 'optimal': 152970},
        'u1817.tsp': {'cities': 1817, 'optimal': 57201},
        'u2152.tsp': {'cities': 2152, 'optimal': 64253},
        'u2319.tsp': {'cities': 2319, 'optimal': 234256},
        'usa13509.tsp': {'cities': 13509, 'optimal': 19982859},
        'vm1084.tsp': {'cities': 1084, 'optimal': 239297},
        'vm1748.tsp': {'cities': 1748, 'optimal': 336556},
    }
