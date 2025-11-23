#!/usr/bin/env python3
"""Analyze benchmark results and calculate statistics"""

import statistics

def analyze_results(problem_name, optimal, without, with2opt, with3opt):
    """Analyze and print statistics for a benchmark run"""

    print(f"\n{'='*70}")
    print(f"RESULTS: {problem_name} (Optimal: {optimal})")
    print(f"{'='*70}\n")

    def calc_stats(results, label):
        avg = statistics.mean(results)
        std = statistics.stdev(results) if len(results) > 1 else 0
        best = min(results)
        worst = max(results)
        gap_avg = ((avg - optimal) / optimal) * 100
        gap_best = ((best - optimal) / optimal) * 100

        print(f"{label}:")
        print(f"  Average:  {avg:8.2f}  ({gap_avg:+6.2f}% from optimal)")
        print(f"  Best:     {best:8.2f}  ({gap_best:+6.2f}% from optimal)")
        print(f"  Worst:    {worst:8.2f}  ({((worst - optimal) / optimal) * 100:+6.2f}% from optimal)")
        print(f"  Std Dev:  {std:8.2f}")
        print()

        return avg, best, gap_avg

    avg_without, best_without, gap_without = calc_stats(without, "WITHOUT Local Search")
    avg_2opt, best_2opt, gap_2opt = calc_stats(with2opt, "WITH 2-opt Only")
    avg_3opt, best_3opt, gap_3opt = calc_stats(with3opt, "WITH 2-opt + 3-opt")

    print("IMPROVEMENTS:")
    print("-" * 70)
    imp_2opt = ((avg_without - avg_2opt) / avg_without) * 100
    imp_3opt = ((avg_without - avg_3opt) / avg_without) * 100
    imp_3opt_vs_2opt = ((avg_2opt - avg_3opt) / avg_2opt) * 100

    print(f"  2-opt only vs baseline:     {imp_2opt:6.2f}% improvement")
    print(f"  2-opt+3-opt vs baseline:    {imp_3opt:6.2f}% improvement")
    print(f"  2-opt+3-opt vs 2-opt only:  {imp_3opt_vs_2opt:6.2f}% improvement")
    print()

    return {
        'problem': problem_name,
        'optimal': optimal,
        'without': (avg_without, gap_without, best_without),
        '2opt': (avg_2opt, gap_2opt, best_2opt),
        '3opt': (avg_3opt, gap_3opt, best_3opt),
        'improvement': imp_3opt
    }

# Run the analysis
if __name__ == "__main__":
    results = []

    # berlin52
    results.append(analyze_results(
        "berlin52.tsp",
        7542,
        [7851.54, 7674.91, 7748.85, 7676.83, 7915.98],
        [7544.37, 7544.37, 7544.37, 7669.62, 7544.37],
        [7658.96, 7544.37, 7544.37, 7658.96, 7544.37]
    ))

    # eil51
    results.append(analyze_results(
        "eil51.tsp",
        426,
        [453.07, 468.57, 464.09, 481.13, 466.63],
        [444.08, 444.79, 441.92, 467.55, 465.25],
        [446.74, 447.31, 456.77, 445.64, 442.59]
    ))

    # st70
    results.append(analyze_results(
        "st70.tsp",
        675,
        [756.36, 769.44, 730.78, 745.41, 751.70],
        [730.03, 735.91, 695.90, 708.69, 744.43],
        [709.19, 709.08, 724.28, 711.01, 698.67]
    ))

    # Summary table
    print("\n" + "="*90)
    print("SUMMARY TABLE")
    print("="*90)
    print(f"{'Problem':<15} {'Optimal':>8} {'Without LS':>15} {'2-opt Only':>15} {'2-opt+3-opt':>15} {'Improv':>8}")
    print("-"*90)

    for r in results:
        print(f"{r['problem']:<15} {r['optimal']:>8} "
              f"{r['without'][0]:>8.1f}({r['without'][1]:+5.1f}%) "
              f"{r['2opt'][0]:>8.1f}({r['2opt'][1]:+5.1f}%) "
              f"{r['3opt'][0]:>8.1f}({r['3opt'][1]:+5.1f}%) "
              f"{r['improvement']:>7.1f}%")

    print("\nNote: All values are averages over 5 runs")
    print("Configuration: 100 iterations, 30 ants")
    print()
