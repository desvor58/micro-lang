#!/usr/bin/env python3
import argparse
import json
import sys
from pathlib import Path

from benchmark import Aggregate, TESTS, fmt_median, measure, ratio_text, ratio_value

ROOT = Path(__file__).resolve().parent
MICRO = ROOT / "bench_micro_lib32"
LLVM = {"O0": ROOT / "bench_llvm_i386_O0", "O2": ROOT / "bench_llvm_i386_O2"}


def main() -> int:
    parser = argparse.ArgumentParser(description="Native i386 micro/LLVM execution comparison")
    parser.add_argument("-n", "--iterations", type=int, default=100)
    parser.add_argument("-r", "--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.iterations <= 0 or args.repeats <= 0:
        parser.error("iterations and repeats must be positive")

    missing = [str(path) for path in (MICRO, *LLVM.values()) if not path.is_file()]
    if missing:
        for path in missing:
            print(f"ERROR: missing executable: {path}", file=sys.stderr)
        return 2

    rows = []
    aggregates: list[Aggregate] = []
    for workload, _, _, _ in TESTS:
        micro = measure("micro", workload, "x86-32", "plain",
                        [str(MICRO), workload, str(args.iterations), "plain"],
                        args.repeats)
        llvm0 = measure("llvm-i386", workload, "i386", "aot-O0",
                        [str(LLVM["O0"]), workload, str(args.iterations), "O0"],
                        args.repeats)
        llvm2 = measure("llvm-i386", workload, "i386", "aot-O2",
                        [str(LLVM["O2"]), workload, str(args.iterations), "O2"],
                        args.repeats)
        rows.append((workload, micro, llvm0, llvm2))
        aggregates.extend((micro, llvm0, llvm2))

    print("i386 native execution (LLVM is AOT, not MCJIT)")
    print("workload       backend       ready us   first call us   steady us")
    for workload, micro, llvm0, llvm2 in rows:
        for name, result in (("micro", micro), ("LLVM O0", llvm0), ("LLVM O2", llvm2)):
            print(f"{workload:14s} {name:12s} {fmt_median(result, 'ready_us'):>9s} "
                  f"{fmt_median(result, 'first_call_us'):>14s} "
                  f"{fmt_median(result, 'steady_us'):>11s}")
    print("steady ratio micro / LLVM O2")
    for workload, micro, _, llvm2 in rows:
        print(f"{workload:14s} {ratio_text(ratio_value(micro, llvm2, 'steady_us', execution=True), execution=True)}")
    print("MIR i386: unavailable; the vendored MIR generator has no i386 backend.")

    failed = [result for result in aggregates if not result.ok]
    report = {
        "metadata": {
            "micro_target": "x86-32",
            "llvm_target": "i386",
            "mir_target": "unavailable",
            "llvm_mode": "AOT",
            "iterations": args.iterations,
            "repeats": args.repeats,
        },
        "results": [result.summary() for result in aggregates],
    }
    output = ROOT / "results_i386.json"
    output.write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    print(f"raw results: {output}")
    if failed:
        for result in failed:
            print(f"ERROR: {result.backend}/{result.workload}/{result.mode}: {result.error}",
                  file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
