#!/usr/bin/env python3
"""Presentation charts for the VM style comparison.

Two standalone figures built from the same VM sample the API benchmark
already measures: one for the speed of building a function through the
public IR API, one for the speed of running it afterwards. micro is
highlighted and MIR is kept neutral, because the point of the figure is
where this project stands against the reference backends.

    python3 comp/vm_graphs.py            # reads comp/results.json
"""

import json
import sys
from pathlib import Path

COMP = Path(__file__).resolve().parent
RESULTS = COMP / "results.json"
COMPILE_GRAPH = COMP / "results_vm_compile.png"
EXECUTION_GRAPH = COMP / "results_vm_execution.png"

SERIES = (("micro", "plain", "micro"), ("mir", "api-o0", "MIR O0"),
          ("mir", "api-o2", "MIR O2"))
VM_NAME_LABEL = {"micro/plain": "micro", "MIR API/O0": "MIR O0", "MIR API/O2": "MIR O2"}
SERIES_COLOR = {"micro": "#2c5f8a", "MIR O0": "#9aa7b1", "MIR O2": "#cfd6dc"}

STYLE = {
    "font.family": "DejaVu Sans",
    "font.size": 11,
    "axes.edgecolor": "#4a4a4a",
    "axes.labelcolor": "#222222",
    "text.color": "#222222",
    "xtick.color": "#333333",
    "ytick.color": "#333333",
    "axes.grid": False,
    "figure.facecolor": "white",
    "axes.facecolor": "white",
    "savefig.facecolor": "white",
}


def vm_rows(results: list[dict], iterations: int, repeats: int) -> list[dict]:
    rows = []
    for backend, mode, label in SERIES:
        for entry in results:
            if entry["workload"] != "vm_sum" or entry["backend"] != backend or entry["mode"] != mode:
                continue
            rows.append({
                "label": label,
                "compile_us": entry["compile_us"],
                "steady_us": entry["steady_us"],
                "iterations": iterations,
                "repeats": repeats,
            })
            break
    return rows


def rows_from_aggregates(vm_results: list[tuple[str, object]], iterations: int,
                        repeats: int) -> list[dict]:
    rows = []
    for name, aggregate in vm_results:
        label = VM_NAME_LABEL.get(name)
        if label is None:
            continue
        rows.append({
            "label": label,
            "compile_us": aggregate.median("compile_us"),
            "steady_us": aggregate.median("steady_us"),
            "iterations": iterations,
            "repeats": repeats,
        })
    return rows


def factor_text(rows: list[dict], field: str, reference: str) -> str:
    micro = next((row for row in rows if row["label"] == "micro"), None)
    other = next((row for row in rows if row["label"] == reference), None)
    if not micro or not other or not micro[field] or not other[field]:
        return f"micro vs {reference}: n/a"
    factor = other[field] / micro[field]
    if factor >= 1:
        return f"micro is {factor:.1f}\u00d7 faster than {reference}"
    return f"{reference} is {1 / factor:.1f}\u00d7 faster than micro"


def chart(rows: list[dict], field: str, title: str, subtitle: str, verdict: str,
          path: Path, unit: str, decimals: int) -> bool:
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        return False

    labels = [row["label"] for row in rows]
    values = [row[field] if row[field] else 0.0 for row in rows]
    colors = [SERIES_COLOR[label] for label in labels]
    iterations = rows[0]["iterations"] if rows else 0
    repeats = rows[0]["repeats"] if rows else 0

    with plt.rc_context(STYLE):
        fig, axis = plt.subplots(figsize=(10.0, 5.8))
        bars = axis.bar(labels, values, width=0.52, color=colors, zorder=3)

        top = max(values) if values else 0.0
        for bar, value in zip(bars, values):
            axis.text(bar.get_x() + bar.get_width() / 2, bar.get_height() + top * 0.02,
                      f"{value:.{decimals}f} {unit}", ha="center", va="bottom", fontsize=12)

        axis.set_ylim(0, top * 1.28 if top else 1)
        axis.set_ylabel(unit, fontsize=12, labelpad=10)
        axis.tick_params(axis="x", labelsize=13, length=0, pad=8)
        axis.tick_params(axis="y", labelsize=11, length=0)
        axis.yaxis.grid(True, color="#e4e4e4", linewidth=0.8, zorder=0)
        axis.set_axisbelow(True)
        axis.spines["top"].set_visible(False)
        axis.spines["right"].set_visible(False)
        axis.spines["left"].set_visible(False)

        fig.subplots_adjust(left=0.10, right=0.985, top=0.79, bottom=0.17)
        fig.text(0.10, 0.945, verdict, fontsize=13, color=SERIES_COLOR["micro"], fontweight="bold")
        fig.text(0.10, 0.893, title, fontsize=17, fontweight="bold")
        fig.text(0.10, 0.845, subtitle, fontsize=12, color="#555555")
        fig.text(0.10, 0.062,
                 "Same function built through each public IR API. "
                 f"Median of {repeats} helper runs of {iterations} compilations; "
                 "execution is the median warmed call.",
                 fontsize=9, color="#666666")
        fig.text(0.10, 0.028,
                 "micro emits x86-32 code, the MIR JIT runs x86-64, so the columns are not "
                 "the same instruction set.",
                 fontsize=9, color="#666666")
        fig.savefig(path, dpi=200)
        plt.close(fig)
    return True


def render(rows: list[dict], compile_path: Path = COMPILE_GRAPH,
           execution_path: Path = EXECUTION_GRAPH) -> bool:
    if not rows:
        return False
    ok = chart(rows, "compile_us",
               "Compilation speed, VM style build",
               "micro API against MIR API, same workload",
               f"{verdict_text(rows, 'compile_us')}",
               compile_path, "\u00b5s", 1)
    ok &= chart(rows, "steady_us",
                "Execution speed of the compiled function",
                "Warmed call of the function the API just built",
                f"{verdict_text(rows, 'steady_us')}",
                execution_path, "\u00b5s", 4)
    return ok


def verdict_text(rows: list[dict], field: str) -> str:
    first = factor_text(rows, field, "MIR O0")
    second = factor_text(rows, field, "MIR O2")
    return f"{first} and {second}"


def main() -> int:
    if not RESULTS.is_file():
        print(f"no {RESULTS}, run python3 comp/benchmark.py first", file=sys.stderr)
        return 2
    report = json.loads(RESULTS.read_text())
    metadata = report.get("metadata", {})
    rows = vm_rows(report["results"], metadata.get("iterations", 0), metadata.get("repeats", 0))
    if len(rows) != len(SERIES):
        present = {row["label"] for row in rows}
        missing = [label for _, _, label in SERIES if label not in present]
        print(f"results.json is missing {missing}, run the full benchmark again", file=sys.stderr)
        return 2
    if not render(rows):
        print("matplotlib is not available", file=sys.stderr)
        return 2
    print(f"compile:  {COMPILE_GRAPH}")
    print(f"execution: {EXECUTION_GRAPH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())