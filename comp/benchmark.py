#!/usr/bin/env python3
import argparse
import json
import math
import os
import platform
import statistics
import subprocess
import sys
import tempfile
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path

import vm_graphs

ROOT = Path(__file__).resolve().parent.parent
COMP = ROOT / "comp"
MICROC = ROOT / "bin" / "microc"
MICRO_API = COMP / "bench_micro_lib32"
LLVM_API = COMP / "bench_llvm_lib"
MIR_API = COMP / "bench_mir_lib"
MIR_VM_API = COMP / "bench_mir_api"

TESTS = [
    ("fibonacci", "fibonacci.micro", "fibonacci.ll", "fibonacci.mir"),
    ("bubblesort", "bubblesort.micro", "bubblesort.ll", "bubblesort.mir"),
    ("matrixmul", "matrixmul.micro", "matrixmul.ll", "matrixmul.mir"),
    ("gcd", "gcd.micro", "gcd.ll", "gcd.mir"),
    ("smoke", "smoke.micro", "smoke.ll", "smoke.mir"),
]


@dataclass
class Sample:
    backend: str
    workload: str
    target: str
    mode: str
    requested: int
    success: int
    opt: str
    compile_us: float
    ready_us: float
    first_call_us: float
    steady_us: float
    cleanup_us: float
    code_size: int
    error: str = ""
    vmopt_us: float = 0.0
    load_us: float = 0.0


@dataclass
class Aggregate:
    backend: str
    workload: str
    target: str
    mode: str
    opt: str
    samples: list[Sample] = field(default_factory=list)
    error: str = ""

    @property
    def ok(self) -> bool:
        return not self.error and bool(self.samples) and all(not s.error for s in self.samples)

    def values(self, field_name: str) -> list[float]:
        return [float(getattr(s, field_name)) for s in self.samples
                if getattr(s, field_name) == getattr(s, field_name)
                and getattr(s, field_name) >= 0]

    def median(self, field_name: str) -> float:
        values = self.values(field_name)
        return statistics.median(values) if values else float("nan")

    def spread(self, field_name: str) -> float:
        values = self.values(field_name)
        return (max(values) - min(values)) if len(values) > 1 else 0.0

    def summary(self) -> dict:
        result = asdict(self)
        result.pop("samples")
        for name in ("compile_us", "ready_us", "load_us", "first_call_us", "steady_us", "cleanup_us", "vmopt_us"):
            value = self.median(name)
            result[name] = None if value != value else value
            result[name + "_spread"] = self.spread(name)
        return result


def run_cmd(cmd: list[str], timeout: float = 120.0) -> tuple[int, str, str]:
    try:
        completed = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
        return completed.returncode, completed.stdout, completed.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "timeout"
    except OSError as exc:
        return -2, "", str(exc)


def parse_kv(backend: str, workload: str, target: str, mode: str,
             output: str) -> Sample:
    values = {}
    for token in output.strip().split():
        if "=" not in token:
            continue
        key, value = token.split("=", 1)
        values[key] = value
    if values.get("schema") != "1" or values.get("status") != "ok":
        return Sample(backend, workload, target, mode, 0, 0, "", -1, -1, -1, -1, -1, 0,
                      values.get("error", "invalid helper output"))
    try:
        requested = int(values.get("requested", "0"))
        success = int(values.get("success", "0"))
        compile_us = float(values.get("compile_us", "-1"))
        ready_us = float(values.get("ready_us", "-1"))
        first_call_us = float(values.get("first_call_us", "-1"))
        steady_us = float(values.get("steady_us", "-1"))
        cleanup_us = float(values.get("cleanup_us", "-1"))
        code_size = int(values.get("code_size", "0"))
        vmopt_us = float(values.get("vmopt_us", "-1"))
        load_us = float(values.get("load_us", "-1"))
    except ValueError as exc:
        return Sample(backend, workload, target, mode, 0, 0, "", -1, -1, -1, -1, -1, 0,
                      f"invalid numeric field: {exc}")
    numeric_values = (compile_us, ready_us, first_call_us, steady_us, cleanup_us, vmopt_us, load_us)
    if not all(math.isfinite(value) for value in numeric_values):
        return Sample(backend, workload, target, mode, requested, success, "",
                      compile_us, ready_us, first_call_us, steady_us, cleanup_us,
                      code_size, "non-finite timing")
    if requested <= 0 or success != requested:
        return Sample(backend, workload, target, mode, requested, success, "",
                      compile_us, ready_us, first_call_us, steady_us, cleanup_us,
                      code_size, "helper reported incomplete compilation")
    return Sample(backend, workload, target, mode, requested, success,
                  values.get("opt", values.get("mode", "")), compile_us, ready_us,
                  first_call_us, steady_us, cleanup_us, code_size,
                  vmopt_us=vmopt_us, load_us=load_us)


def measure(backend: str, workload: str, target: str, mode: str,
            command: list[str], repeats: int) -> Aggregate:
    aggregate = Aggregate(backend, workload, target, mode, "")
    rc, _, stderr = run_cmd(command)
    if rc != 0:
        aggregate.error = stderr.strip() or f"warmup exit status {rc}"
        return aggregate
    for _ in range(repeats):
        rc, stdout, stderr = run_cmd(command)
        if rc != 0:
            aggregate.error = stderr.strip() or f"exit status {rc}"
            return aggregate
        sample = parse_kv(backend, workload, target, mode, stdout)
        if sample.error:
            aggregate.error = sample.error
            return aggregate
        aggregate.samples.append(sample)
        aggregate.opt = sample.opt
    return aggregate


def fmt(value: float) -> str:
    if value != value or value < 0:
        return "n/a"
    if value < 10:
        return f"{value:.2f}"
    return f"{value:.1f}"


def fmt_stat(aggregate: Aggregate, field_name: str) -> str:
    value = aggregate.median(field_name)
    if value != value or value < 0:
        return "n/a"
    spread = aggregate.spread(field_name)
    return f"{fmt(value)} ± {fmt(spread)}"


def fmt_median(result: Aggregate, field_name: str) -> str:
    value = result.median(field_name)
    return fmt(value)


def check_file(path: Path) -> bool:
    return path.is_file() and os.access(path, os.X_OK)


def elf_class(path: Path) -> str:
    try:
        with path.open("rb") as file:
            header = file.read(6)
        if header[:4] != b"\x7fELF":
            return "unknown"
        return {1: "ELF32", 2: "ELF64"}.get(header[4], "unknown")
    except OSError:
        return "unknown"


def prerequisites(need_microc: bool) -> list[str]:
    errors = []
    paths = [MICRO_API, LLVM_API, MIR_API, MIR_VM_API]
    if need_microc:
        paths.append(MICROC)
    for path in paths:
        if not check_file(path):
            errors.append(f"missing executable: {path}")
    if check_file(MICRO_API) and elf_class(MICRO_API) != "ELF32":
        errors.append(f"micro API helper must be ELF32: {MICRO_API}")
    for path in (LLVM_API, MIR_API, MIR_VM_API):
        if check_file(path) and elf_class(path) != "ELF64":
            errors.append(f"native API helper must be ELF64: {path}")
    return errors


def cli_compile(name: str, source: str, iterations: int, repeats: int) -> Aggregate:
    aggregate = Aggregate("cli", name, "native", "cli", "")
    source_path = str(COMP / source)
    with tempfile.TemporaryDirectory(prefix="micro-bench-") as directory:
        if source.endswith(".micro"):
            command = [str(MICROC), "-o", os.path.join(directory, name + ".bin"), source_path]
        elif source.endswith(".ll"):
            command = ["clang", "-O0", "-c", source_path, "-o", os.path.join(directory, name + ".o")]
        else:
            raise RuntimeError(f"no command line compile path for {source}")
        for _ in range(repeats):
            start = time.perf_counter()
            rc, _, stderr = run_cmd(command)
            elapsed = (time.perf_counter() - start) * 1e6
            if rc != 0:
                aggregate.error = stderr.strip() or f"exit status {rc}"
                return aggregate
            aggregate.samples.append(Sample(
                "cli", name, "native", "cli", iterations, iterations, "O0",
                elapsed, elapsed, -1, -1, -1, 0))
    return aggregate


def print_api_table(results: list[tuple[str, Aggregate, Aggregate, Aggregate]]) -> None:
    print("API compilation (median ± spread, µs; micro x86-32, LLVM/MIR x86-64)")
    print("workload       micro/x86-32   LLVM/x86-64    MIR/x86-64")
    for name, micro, llvm, mir in results:
        print(f"{name:14s} {fmt_stat(micro, 'compile_us'):>13s} "
              f"{fmt_stat(llvm, 'compile_us'):>13s} "
              f"{fmt_stat(mir, 'compile_us'):>13s}")
    print("API JIT-ready, first call and steady execution (median ± spread, µs; mixed ISA)")
    print("workload       micro ready   micro first   micro call   LLVM ready   LLVM first   LLVM call   MIR ready   MIR first   MIR call")
    for name, micro, llvm, mir in results:
        print(f"{name:14s} {fmt_stat(micro, 'ready_us'):>11s} "
              f"{fmt_stat(micro, 'first_call_us'):>11s} "
              f"{fmt_stat(micro, 'steady_us'):>11s} "
              f"{fmt_stat(llvm, 'ready_us'):>11s} "
              f"{fmt_stat(llvm, 'first_call_us'):>11s} "
              f"{fmt_stat(llvm, 'steady_us'):>11s} "
              f"{fmt_stat(mir, 'ready_us'):>11s} "
              f"{fmt_stat(mir, 'first_call_us'):>11s} "
              f"{fmt_stat(mir, 'steady_us'):>11s}")


def print_vm_table(results: list[tuple[str, Aggregate]]) -> None:
    print("VM API comparison (micro x86-32; MIR x86-64)")
    print("backend/mode                 ready us   load us   first call us   steady us   vmopt us   code bytes")
    for name, result in results:
        code_size = result.samples[0].code_size if result.samples else 0
        size = str(code_size) if code_size else "n/a"
        print(f"{name:27s} {fmt_stat(result, 'ready_us'):>9s} "
              f"{fmt_stat(result, 'load_us'):>8s} "
              f"{fmt_stat(result, 'first_call_us'):>14s} "
              f"{fmt_stat(result, 'steady_us'):>11s} "
              f"{fmt_stat(result, 'vmopt_us'):>9s} {size:>11s}")


def ratio_value(micro: Aggregate, mir: Aggregate, field_name: str,
                execution: bool = False) -> float:
    micro_value = micro.median(field_name)
    mir_value = mir.median(field_name)
    if micro_value <= 0 or mir_value <= 0 or micro_value != micro_value or mir_value != mir_value:
        return float("nan")
    return micro_value / mir_value if execution else mir_value / micro_value


def ratio_text(value: float, execution: bool = False) -> str:
    if value != value or value <= 0:
        return "n/a"
    if execution:
        if value >= 1:
            return f"{value:.2f}× slower"
        return f"{1 / value:.2f}× faster"
    if value >= 1:
        return f"{value:.2f}× faster"
    return f"{1 / value:.2f}× slower"


BACKEND_COLORS = {"micro": "#e45756", "llvm": "#4c78a8", "mir": "#f58518"}
WIN_COLOR = "#54a24b"
LOSE_COLOR = "#e45756"

METRIC_TITLES = {
    "compile_us": "Compilation",
    "ready_us": "Ready to call",
    "first_call_us": "First call",
    "steady_us": "Warmed execution",
    "cleanup_us": "Teardown",
}


def plain_log_axis(axis, np, span: float = 0.0) -> None:
    """Log axis with readable plain numbers instead of 10^n tick clutter."""
    from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter
    axis.set_xscale("log")
    subs = (1.0, 3.0) if 0.0 < span < 1e4 else (1.0,)
    axis.xaxis.set_major_locator(LogLocator(base=10.0, subs=subs))
    axis.xaxis.set_minor_formatter(NullFormatter())
    axis.xaxis.set_major_formatter(FuncFormatter(lambda v, _: f"{v:.3g}"))


def median_of(result: Aggregate, field_name: str) -> float:
    value = result.median(field_name)
    return value if value > 0 and value == value else float("nan")


def speed_factor(micro: Aggregate, other: Aggregate, field_name: str) -> float:
    """other / micro for a metric where a smaller value is better.

    Above 1.0 means micro is the faster one, below 1.0 means the other
    backend is.
    """
    micro_value = median_of(micro, field_name)
    other_value = median_of(other, field_name)
    if micro_value != micro_value or other_value != other_value:
        return float("nan")
    return other_value / micro_value


def factor_caption(value: float) -> str:
    if value != value:
        return "n/a"
    if value >= 1:
        return f"{value:.1f}\u00d7 faster"
    return f"{1 / value:.1f}\u00d7 slower"


def factor_panel(axis, field_name: str, api_results, plt, np) -> None:
    labels, values = [], []
    for name, micro, llvm, mir in api_results:
        labels.append(f"{name}\nvs LLVM")
        values.append(speed_factor(micro, llvm, field_name))
        labels.append(f"{name}\nvs MIR")
        values.append(speed_factor(micro, mir, field_name))

    y = np.arange(len(values))
    finite = [(i, v) for i, v in enumerate(values) if v == v]
    span_lo = min([v for _, v in finite], default=1.0)
    span_hi = max([v for _, v in finite], default=1.0)
    axis.axvspan(1.0, span_hi * 4, color=WIN_COLOR, alpha=0.14)
    axis.axvspan(span_lo / 8, 1.0, color=LOSE_COLOR, alpha=0.14)
    if finite:
        axis.barh([i for i, _ in finite], [v for _, v in finite],
                  color=[WIN_COLOR if v >= 1.0 else LOSE_COLOR for _, v in finite])
        for i, v in finite:
            axis.text(v * 1.08, i, factor_caption(v), va="center", ha="left", fontsize=8)
    axis.axvline(1.0, color="#333", linewidth=1.2)
    plain_log_axis(axis, np, span=(span_hi * 60) / max(span_lo / 8, 1e-12))
    axis.set_xlim(span_lo / 8, span_hi * 60)
    axis.set_yticks(y)
    axis.set_yticklabels(labels, fontsize=8)
    wins = len([v for _, v in finite if v >= 1.0])
    verdict = f"micro faster in {wins} of {len(finite)} comparisons"
    axis.set_title(f"How much faster micro is \u2014 {METRIC_TITLES[field_name]}\n"
                   f"{verdict}", fontsize=10)
    axis.set_xlabel("factor, 1.0 = equal")
    axis.grid(True, which="both", axis="x", alpha=0.25)
    axis.invert_yaxis()


def absolute_panel(axis, field_name: str, api_results, plt, np) -> None:
    labels, micro_values, llvm_values, mir_values = [], [], [], []
    for name, micro, llvm, mir in api_results:
        labels.append(name)
        micro_values.append(median_of(micro, field_name))
        llvm_values.append(median_of(llvm, field_name))
        mir_values.append(median_of(mir, field_name))

    height = 0.26
    y = np.arange(len(labels))
    for offset, values, key in ((height, llvm_values, "llvm"), (0.0, micro_values, "micro"),
                                (-height, mir_values, "mir")):
        finite = [(i, v) for i, v in enumerate(values) if v == v]
        axis.barh([i + offset for i, _ in finite], [v for _, v in finite], height,
                  color=BACKEND_COLORS[key], label=key)
        for i, v in finite:
            axis.text(v * 1.12, i + offset, f"{v:.3g}", va="center", ha="left", fontsize=7)
    present = [v for v in micro_values + llvm_values + mir_values if v == v]
    plain_log_axis(axis, np, span=(max(present) * 12.0) / min(present) if present else 0.0)
    if present:
        axis.set_xlim(min(present) / 3.0, max(present) * 12.0)
    axis.set_yticks(y)
    axis.set_yticklabels(labels, fontsize=9)
    axis.set_title(f"{METRIC_TITLES[field_name]} \u2014 absolute", fontsize=10)
    axis.set_xlabel("microseconds, lower is better")
    axis.grid(True, which="both", axis="x", alpha=0.25)
    axis.legend(fontsize=8)
    axis.invert_yaxis()


def code_size_panel(axis, api_results, plt, np) -> None:
    labels = [name for name, _, _, _ in api_results]
    values = [result.samples[0].code_size if result.samples else 0
              for _, result, _, _ in api_results]
    y = np.arange(len(labels))
    axis.barh(y, [v if v else 0.001 for v in values], color=BACKEND_COLORS["micro"])
    for i, v in enumerate(values):
        axis.text(v * 1.08 + 1, i, f"{v} B" if v else "n/a", va="center", ha="left", fontsize=8)
    if values:
        axis.set_xlim(0, max(values) * 1.25)
    axis.set_yticks(y)
    axis.set_yticklabels(labels, fontsize=9)
    axis.set_title("Code size emitted by micro", fontsize=10)
    axis.set_xlabel("bytes (only micro reports a size)")
    axis.grid(True, axis="x", alpha=0.25)
    axis.invert_yaxis()


def vm_panel(axis, field_name: str, vm_results, plt, np) -> None:
    labels = [name for name, _ in vm_results]
    values = [median_of(result, field_name) for _, result in vm_results]
    colors = [BACKEND_COLORS["mir"] if name.startswith("MIR") else BACKEND_COLORS["micro"]
              for name in labels]
    y = np.arange(len(labels))
    finite = [(i, v) for i, v in enumerate(values) if v == v]
    axis.barh([i for i, _ in finite], [v for _, v in finite],
              color=[colors[i] for i, _ in finite])
    for i, v in finite:
        axis.text(v * 1.12, i, f"{v:.3g} us", va="center", ha="left", fontsize=8)
    for i, v in enumerate(values):
        if v != v:
            axis.text(0.0001, i, "n/a", va="center", ha="left", fontsize=8, color="#777")
    low = min((v for _, v in finite), default=1.0)
    high = max((v for _, v in finite), default=1.0)
    plain_log_axis(axis, np, span=(high * 12.0) / max(low, 1e-12))
    if finite:
        axis.set_xlim(low / 3.0, high * 12.0)
    axis.set_yticks(y)
    axis.set_yticklabels(labels, fontsize=9)
    axis.set_title(f"VM API \u2014 {METRIC_TITLES[field_name]}", fontsize=10)
    axis.set_xlabel("microseconds, lower is better")
    axis.grid(True, which="both", axis="x", alpha=0.25)
    axis.invert_yaxis()


def plot_vm_results(vm_results: list[tuple[str, Aggregate]], path: Path) -> bool:
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError:
        return False
    if not vm_results:
        return False
    fig, axes = plt.subplots(1, 2, figsize=(15, 5))
    vm_panel(axes[0], "ready_us", vm_results, plt, np)
    vm_panel(axes[1], "steady_us", vm_results, plt, np)
    fig.suptitle("micro / MIR VM API comparison", fontsize=15, fontweight="bold")
    fig.text(0.5, 0.01, "micro is x86-32; MIR is x86-64. Log scale; n/a is never plotted as zero.",
             ha="center", fontsize=9)
    fig.tight_layout(rect=(0, 0.04, 1, 0.94))
    fig.savefig(path, dpi=160, bbox_inches="tight")
    plt.close(fig)
    return True


def plot_results(api_results: list[tuple[str, Aggregate, Aggregate, Aggregate]],
                 vm_results: list[tuple[str, Aggregate]], path: Path) -> bool:
    if not api_results:
        return plot_vm_results(vm_results, path)
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
    except ImportError:
        return False

    fig, axes = plt.subplots(4, 3, figsize=(20, 20))

    def factor(row: int, column: int, field_name: str) -> None:
        factor_panel(axes[row, column], field_name, api_results, plt, np)

    def absolute(row: int, column: int, field_name: str) -> None:
        absolute_panel(axes[row, column], field_name, api_results, plt, np)

    # row 0: how much faster, the headline question
    factor(0, 0, "compile_us")
    factor(0, 1, "ready_us")
    factor(0, 2, "steady_us")
    # row 1: the same three metrics in absolute microseconds
    absolute(1, 0, "compile_us")
    absolute(1, 1, "ready_us")
    absolute(1, 2, "steady_us")
    # row 2: the remaining API phases and the emitted code size
    factor(2, 0, "first_call_us")
    factor(2, 1, "cleanup_us")
    code_size_panel(axes[2, 2], api_results, plt, np)
    # row 3: first call in absolute terms and the VM style comparison
    absolute(3, 0, "first_call_us")
    if vm_results:
        vm_panel(axes[3, 1], "ready_us", vm_results, plt, np)
        vm_panel(axes[3, 2], "steady_us", vm_results, plt, np)
    else:
        for column in (1, 2):
            axes[3, column].text(0.5, 0.5, "no VM results", ha="center", va="center",
                                 transform=axes[3, column].transAxes)
            axes[3, column].set_axis_off()

    fig.suptitle("micro vs LLVM vs MIR \u2014 who is faster, by how much, and in what",
                 fontsize=17, fontweight="bold")
    fig.text(0.5, 0.012,
             "Rows 1 and 2: micro against LLVM and against MIR, one chart per metric \u2014 "
             "row 1 is the factor, row 2 the same metric in absolute microseconds. "
             "In the factor charts left of the 1.0 line the other backend wins, right of it micro wins. "
             "Row 3 adds the first call, the teardown and the emitted code size, row 4 the VM style "
             "comparison. micro is x86-32, LLVM and MIR are x86-64, so the ratios mix instruction sets; "
             "MIR has no i386 backend. Log scale; n/a is never plotted as zero.",
             ha="center", fontsize=10)
    fig.tight_layout(rect=(0, 0.03, 1, 0.96))
    fig.savefig(path, dpi=160, bbox_inches="tight")
    plt.close(fig)
    return True


def render_report(api_results: list[tuple[str, Aggregate, Aggregate, Aggregate]],
                  vm_results: list[tuple[str, Aggregate]],
                  cli_rows: list[tuple[str, list[Aggregate]]],
                  metadata: dict, graph_path: Path) -> None:
    try:
        from rich.console import Console
        from rich.panel import Panel
        from rich.table import Table
    except ImportError:
        print("CLI command latency (separate from API/JIT measurements)")
        for name, values in cli_rows:
            print(f"{name:14s} " + "  ".join(fmt_stat(value, "compile_us") for value in values))
        print_api_table(api_results)
        print_vm_table(vm_results)
        return

    console = Console()
    if cli_rows:
        table = Table(title="⏱ CLI command latency", show_lines=True)
        table.add_column("Workload", style="bold cyan")
        table.add_column("microc", justify="right")
        table.add_column("clang", justify="right")
        for name, values in cli_rows:
            table.add_row(name, fmt_stat(values[0], "compile_us"),
                          fmt_stat(values[1], "compile_us"))
        console.print(table)

    if api_results:
        table = Table(title="📚 API compilation (median ± spread, µs; mixed ISA)", show_lines=True)
        table.add_column("Workload", style="bold cyan")
        for backend in ("micro\nx86-32", "LLVM\nx86-64", "MIR\nx86-64"):
            table.add_column(backend, justify="right")
        for name, micro, llvm, mir in api_results:
            table.add_row(name, fmt_stat(micro, "compile_us"),
                          fmt_stat(llvm, "compile_us"), fmt_stat(mir, "compile_us"))
        console.print(table)

        table = Table(title="⚡ API execution: ready / first / steady (median, µs; mixed ISA)", show_lines=True)
        table.add_column("Workload", style="bold cyan")
        for backend in ("micro\nx86-32", "LLVM\nx86-64", "MIR\nx86-64"):
            table.add_column(f"{backend} r/f/s", justify="right")
        for name, micro, llvm, mir in api_results:
            table.add_row(name,
                          " / ".join((fmt_median(micro, "ready_us"),
                                      fmt_median(micro, "first_call_us"),
                                      fmt_median(micro, "steady_us"))),
                          " / ".join((fmt_median(llvm, "ready_us"),
                                      fmt_median(llvm, "first_call_us"),
                                      fmt_median(llvm, "steady_us"))),
                          " / ".join((fmt_median(mir, "ready_us"),
                                      fmt_median(mir, "first_call_us"),
                                      fmt_median(mir, "steady_us"))))
        console.print(table)

        table = Table(title="⚖ micro i386 vs MIR x86-64 (mixed ISA)", show_lines=True)
        table.add_column("Workload", style="bold cyan")
        table.add_column("compile", justify="right")
        table.add_column("steady execution", justify="right")
        for name, micro, _, mir in api_results:
            table.add_row(name,
                          ratio_text(ratio_value(micro, mir, "compile_us")),
                          ratio_text(ratio_value(micro, mir, "steady_us", execution=True),
                                     execution=True))
        console.print(table)

    if vm_results:
        table = Table(title="🧬 VM API comparison (micro x86-32; MIR x86-64)", show_lines=True)
        table.add_column("Backend/mode", style="bold cyan", no_wrap=True)
        table.add_column("ready", justify="right")
        table.add_column("load", justify="right")
        table.add_column("first call", justify="right")
        table.add_column("steady", justify="right")
        table.add_column("vmopt", justify="right")
        table.add_column("code bytes", justify="right")
        for name, result in vm_results:
            code_size = result.samples[0].code_size if result.samples else 0
            table.add_row(name, fmt_median(result, "ready_us"), fmt_median(result, "load_us"),
                          fmt_median(result, "first_call_us"), fmt_median(result, "steady_us"),
                          fmt_median(result, "vmopt_us"), str(code_size) if code_size else "n/a")
        console.print(table)

    lines = [
        f"Platform: {metadata['platform']}",
        f"micro target: {metadata['micro_target']} | LLVM target: x86-64 | MIR target: x86-64",
        "MIR i386: unsupported in the vendored backend; LLVM i386 uses separate AOT helpers.",
        "Native ratios are mixed-ISA and are not an ISA-independent code-quality claim.",
        f"iterations={metadata['iterations']} | repeats={metadata['repeats']}",
        f"graph: {graph_path}",
    ]
    console.print(Panel("\n".join(lines), title="📊 Summary", border_style="blue"))


def main() -> int:
    parser = argparse.ArgumentParser(description="Objective micro/LLVM/MIR API benchmark")
    parser.add_argument("-n", "--iterations", type=int, default=100)
    parser.add_argument("-r", "--repeats", type=int, default=3)
    parser.add_argument("--hints-only", action="store_true")
    parser.add_argument("--no-cli", action="store_true")
    args = parser.parse_args()
    if args.iterations <= 0 or args.repeats <= 0:
        parser.error("iterations and repeats must be positive")

    errors = prerequisites(not args.no_cli and not args.hints_only)
    if errors:
        for error in errors:
            print(f"ERROR: {error}", file=sys.stderr)
        return 2

    metadata = {
        "timestamp": time.time(),
        "platform": platform.platform(),
        "python": sys.version,
        "micro_target": elf_class(MICRO_API),
        "native_target": elf_class(MIR_API),
        "llvm_target": "x86-64",
        "mir_target": "x86-64",
        "mir_i386": "unsupported: vendored MIR has no i386 native backend",
        "llvm_i386": "separate AOT helpers when llc and a 32-bit toolchain are available; ELF64 MCJIT cannot execute i386",
        "isa_warning": "micro is x86-32; LLVM and MIR are x86-64",
        "iterations": args.iterations,
        "repeats": args.repeats,
    }
    aggregates: list[Aggregate] = []
    cli_rows: list[tuple[str, list[Aggregate]]] = []
    api_results: list[tuple[str, Aggregate, Aggregate, Aggregate]] = []

    if not args.hints_only and not args.no_cli:
        # MIR has no command line compile path here: the vendored MIR text is
        # consumed through the library, so the row is measured by the API helper.
        for name, micro_file, ll_file, _ in TESTS:
            cli_results = [
                cli_compile(name, micro_file, args.iterations, args.repeats),
                cli_compile(name, ll_file, args.iterations, args.repeats),
            ]
            aggregates.extend(cli_results)
            cli_rows.append((name, cli_results))

    if not args.hints_only:
        for name, _, ll_file, mir_file in TESTS:
            micro = measure("micro", name, "x86-32", "plain",
                            [str(MICRO_API), name, str(args.iterations), "plain"],
                            args.repeats)
            llvm = measure("llvm", name, "x86-64", "mcjit",
                           [str(LLVM_API), str(COMP / ll_file), str(args.iterations)],
                           args.repeats)
            mir = measure("mir", name, "x86-64", "api-o2",
                          [str(MIR_API), str(COMP / mir_file), str(args.iterations), "2"],
                          args.repeats)
            aggregates.extend((micro, llvm, mir))
            api_results.append((name, micro, llvm, mir))

    vm_results = []
    for mode in ("plain", "hinted"):
        result = measure("micro", "vm_sum", "x86-32", mode,
                         [str(MICRO_API), "hints", str(args.iterations), mode],
                         args.repeats)
        aggregates.append(result)
        vm_results.append((f"micro/{mode}", result))
    for level in (0, 1, 2, 3):
        result = measure("mir", "vm_sum", "x86-64", f"api-o{level}",
                         [str(MIR_VM_API), str(args.iterations), str(level)],
                         args.repeats)
        aggregates.append(result)
        vm_results.append((f"MIR API/O{level}", result))
    graph_path = COMP / "results.png"
    graph_ok = plot_results(api_results, vm_results, graph_path)
    metadata["graph"] = str(graph_path) if graph_ok else None

    vm_rows = vm_graphs.rows_from_aggregates(vm_results, args.iterations, args.repeats)
    vm_graphs.render(vm_rows)
    metadata["vm_compile_graph"] = str(vm_graphs.COMPILE_GRAPH) if vm_rows else None
    metadata["vm_execution_graph"] = str(vm_graphs.EXECUTION_GRAPH) if vm_rows else None
    render_report(api_results, vm_results, cli_rows, metadata, graph_path)

    failed = [result for result in aggregates if not result.ok]
    report = {
        "metadata": metadata,
        "results": [result.summary() for result in aggregates],
        "raw": [asdict(sample) for result in aggregates for sample in result.samples],
    }
    output = COMP / "results.json"
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
