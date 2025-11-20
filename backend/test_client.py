#!/usr/bin/env python3
"""
Test client for ACO Solver Flask API

Tests both REST endpoints and WebSocket functionality.
Run the Flask server first: python app.py
Then run this test: python test_client.py
"""

import socketio
import requests
import time
import sys

# Server URL
BASE_URL = "http://localhost:5000"
API_URL = f"{BASE_URL}/api"

def test_rest_endpoints():
    """Test REST API endpoints"""
    print("=" * 60)
    print("Testing REST Endpoints")
    print("=" * 60)

    # Test health check
    print("\n1. Testing /api/health...")
    response = requests.get(f"{API_URL}/health")
    print(f"   Status: {response.status_code}")
    print(f"   Response: {response.json()}")
    assert response.status_code == 200
    assert response.json()['status'] == 'healthy'
    print("   ✓ Health check passed")

    # Test benchmarks list
    print("\n2. Testing /api/benchmarks...")
    response = requests.get(f"{API_URL}/benchmarks")
    print(f"   Status: {response.status_code}")
    data = response.json()
    print(f"   Found {data['count']} benchmarks")
    assert response.status_code == 200
    assert data['count'] > 0
    print(f"   First benchmark: {data['benchmarks'][0]['name']} ({data['benchmarks'][0]['cities']} cities)")
    print("   ✓ Benchmarks list passed")

    # Test specific benchmark
    print("\n3. Testing /api/benchmarks/berlin52.tsp...")
    response = requests.get(f"{API_URL}/benchmarks/berlin52.tsp")
    print(f"   Status: {response.status_code}")
    data = response.json()
    print(f"   Benchmark: {data['name']}, Cities: {data['cities']}, Optimal: {data['optimal']}")
    assert response.status_code == 200
    assert data['cities'] == 52
    print("   ✓ Specific benchmark passed")

    # Test parameters
    print("\n4. Testing /api/parameters...")
    response = requests.get(f"{API_URL}/parameters")
    print(f"   Status: {response.status_code}")
    data = response.json()
    print(f"   Default parameters: {data['parameters']}")
    assert response.status_code == 200
    assert 'numAnts' in data['parameters']
    print("   ✓ Parameters passed")

    print("\n✓ All REST endpoint tests passed!\n")


def test_websocket():
    """Test WebSocket solve functionality"""
    print("=" * 60)
    print("Testing WebSocket Solve")
    print("=" * 60)

    # Create SocketIO client
    sio = socketio.Client()

    # Track events
    events = {
        'connected': False,
        'loaded': False,
        'progress_count': 0,
        'complete': False,
        'error': None
    }

    results = {}

    @sio.on('connected')
    def on_connected(data):
        events['connected'] = True
        print(f"\n✓ Connected: {data['message']}")

    @sio.on('loaded')
    def on_loaded(data):
        events['loaded'] = True
        print(f"✓ Loaded: {data['benchmark']} with {data['numCities']} cities")

    @sio.on('progress')
    def on_progress(data):
        events['progress_count'] += 1
        iteration = data['iteration']
        best_distance = data['bestDistance']
        progress = data['progress']
        elapsed = data['elapsedTime']
        print(f"  Progress: Iteration {iteration:3d} - Distance: {best_distance:8.2f} - "
              f"Progress: {progress:5.1f}% - Elapsed: {elapsed:.2f}s")

    @sio.on('complete')
    def on_complete(data):
        events['complete'] = True
        results.update(data)
        print(f"\n✓ Complete!")
        print(f"  Best distance: {data['bestDistance']:.2f}")
        print(f"  Total iterations: {data['totalIterations']}")
        print(f"  Elapsed time: {data['elapsedTime']:.2f}s")
        print(f"  Improvement: {data['convergenceHistory'][0] - data['bestDistance']:.2f}")

    @sio.on('error')
    def on_error(data):
        events['error'] = data['message']
        print(f"\n✗ Error: {data['message']}")

    try:
        # Connect to server
        print("\nConnecting to server...")
        sio.connect(BASE_URL)
        time.sleep(0.5)

        assert events['connected'], "Failed to connect"

        # Send solve request
        print("\nSending solve request for berlin52.tsp (50 iterations)...")
        sio.emit('solve', {
            'benchmark': 'berlin52.tsp',
            'params': {
                'numAnts': 20,
                'iterations': 50,
                'alpha': 1.0,
                'beta': 2.0,
                'rho': 0.5,
                'Q': 100.0
            }
        })

        # Wait for completion (max 30 seconds)
        wait_time = 0
        while not events['complete'] and wait_time < 30:
            time.sleep(0.5)
            wait_time += 0.5

        # Verify results
        assert events['loaded'], "Graph was not loaded"
        assert events['progress_count'] > 0, "No progress updates received"
        assert events['complete'], "Optimization did not complete"
        assert events['error'] is None, f"Error occurred: {events['error']}"

        print(f"\n✓ Received {events['progress_count']} progress updates")
        print(f"✓ Final result: {results['bestDistance']:.2f} (optimal: 7542)")

        # Disconnect
        sio.disconnect()
        print("\n✓ Disconnected")

        print("\n✓ All WebSocket tests passed!")

    except Exception as e:
        print(f"\n✗ WebSocket test failed: {e}")
        import traceback
        traceback.print_exc()
        sio.disconnect()
        return False

    return True


def main():
    """Run all tests"""
    print("\n" + "=" * 60)
    print("ACO Solver Flask API - Test Suite")
    print("=" * 60)
    print("\nMake sure the Flask server is running:")
    print("  python app.py")
    print("\nStarting tests in 2 seconds...\n")
    time.sleep(2)

    try:
        # Test REST endpoints
        test_rest_endpoints()

        # Test WebSocket
        success = test_websocket()

        if success:
            print("\n" + "=" * 60)
            print("✓ ALL TESTS PASSED")
            print("=" * 60)
            return 0
        else:
            print("\n" + "=" * 60)
            print("✗ SOME TESTS FAILED")
            print("=" * 60)
            return 1

    except requests.ConnectionError:
        print("\n✗ Cannot connect to server. Make sure Flask server is running:")
        print("  python app.py")
        return 1

    except Exception as e:
        print(f"\n✗ Test failed with error: {e}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
