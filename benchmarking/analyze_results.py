#!/usr/bin/env python3
"""
Quick Analysis Script for Benchmark Results

Usage:
    python analyze_results.py results/benchmark_YYYYMMDD_HHMMSS.csv
"""

import sys
import pandas as pd
import argparse


def analyze_benchmark(csv_file):
    """Analyze benchmark results and print summary statistics"""

    print("=" * 80)
    print("  ACO TSP Solver - Benchmark Analysis")
    print("=" * 80)
    print()

    # Load data
    df = pd.read_csv(csv_file)
    print(f"Loaded {len(df)} benchmark results from: {csv_file}")
    print()

    # Summary by problem and threads
    summary = df.groupby(['Problem', 'Threads']).agg({
        'Time(ms)': ['mean', 'std', 'min', 'max'],
        'BestDistance': ['mean', 'std'],
        'Gap(%)': ['mean', 'std']
    }).round(2)

    # Calculate speedups
    print("-" * 80)
    print("SPEEDUP ANALYSIS (vs Serial)")
    print("-" * 80)
    print()

    for problem in df['Problem'].unique():
        problem_data = summary.loc[problem]

        # Get serial time
        if 1 not in problem_data.index:
            print(f"⚠️  {problem}: No serial baseline found")
            continue

        serial_time = problem_data.loc[1, ('Time(ms)', 'mean')]

        print(f"{problem}")
        print(f"  Serial baseline: {serial_time:.2f}ms")
        print()
        print(f"  {'Threads':>10} {'Avg Time (ms)':>15} {'Speedup':>10} {'Efficiency':>12}")
        print(f"  {'-'*10} {'-'*15} {'-'*10} {'-'*12}")

        for threads in sorted(problem_data.index):
            avg_time = problem_data.loc[threads, ('Time(ms)', 'mean')]
            speedup = serial_time / avg_time if avg_time > 0 else 0
            efficiency = (speedup / threads * 100) if threads > 0 else 100

            threads_label = "auto" if threads == 0 else str(threads)

            print(f"  {threads_label:>10} {avg_time:>15.2f} {speedup:>9.2f}× {efficiency:>11.1f}%")

        print()

    # Overall statistics
    print("-" * 80)
    print("SOLUTION QUALITY")
    print("-" * 80)
    print()

    quality = df.groupby('Problem').agg({
        'Gap(%)': ['mean', 'std', 'min', 'max']
    }).round(2)

    print(f"{'Problem':20} {'Avg Gap':>10} {'Std Dev':>10} {'Min Gap':>10} {'Max Gap':>10}")
    print(f"{'-'*20} {'-'*10} {'-'*10} {'-'*10} {'-'*10}")

    for problem in quality.index:
        avg_gap = quality.loc[problem, ('Gap(%)', 'mean')]
        std_gap = quality.loc[problem, ('Gap(%)', 'std')]
        min_gap = quality.loc[problem, ('Gap(%)', 'min')]
        max_gap = quality.loc[problem, ('Gap(%)', 'max')]

        print(f"{problem:20} {avg_gap:>9.2f}% {std_gap:>9.2f}% {min_gap:>9.2f}% {max_gap:>9.2f}%")

    print()

    # Thread scaling efficiency
    print("-" * 80)
    print("SCALING EFFICIENCY (Average across all problems)")
    print("-" * 80)
    print()

    # Calculate average speedup for each thread count
    speedups = []
    for problem in df['Problem'].unique():
        problem_df = df[df['Problem'] == problem]
        problem_summary = problem_df.groupby('Threads')['Time(ms)'].mean()

        if 1 in problem_summary.index:
            serial_time = problem_summary[1]
            for threads in problem_summary.index:
                if threads != 1:
                    speedup = serial_time / problem_summary[threads]
                    speedups.append({
                        'threads': threads,
                        'speedup': speedup,
                        'efficiency': (speedup / threads * 100) if threads > 0 else 0
                    })

    if speedups:
        speedup_df = pd.DataFrame(speedups)
        thread_stats = speedup_df.groupby('threads').agg({
            'speedup': ['mean', 'std'],
            'efficiency': 'mean'
        }).round(2)

        print(f"{'Threads':>10} {'Avg Speedup':>12} {'Std Dev':>10} {'Efficiency':>12}")
        print(f"{'-'*10} {'-'*12} {'-'*10} {'-'*12}")

        for threads in sorted(thread_stats.index):
            avg_speedup = thread_stats.loc[threads, ('speedup', 'mean')]
            std_speedup = thread_stats.loc[threads, ('speedup', 'std')]
            efficiency = thread_stats.loc[threads, ('efficiency', 'mean')]

            threads_label = "auto" if threads == 0 else str(threads)

            print(f"{threads_label:>10} {avg_speedup:>11.2f}× {std_speedup:>9.2f}× {efficiency:>11.1f}%")

    print()
    print("=" * 80)


def main():
    parser = argparse.ArgumentParser(
        description='Analyze ACO TSP benchmark results'
    )
    parser.add_argument(
        'csv_file',
        help='Path to benchmark CSV file'
    )

    args = parser.parse_args()

    try:
        analyze_benchmark(args.csv_file)
    except FileNotFoundError:
        print(f"Error: File not found: {args.csv_file}")
        sys.exit(1)
    except Exception as e:
        print(f"Error analyzing results: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
