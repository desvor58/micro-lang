#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COMP = ROOT / "comp"
MIR = ROOT / "mir"


def run(command: list[str]) -> None:
    print("+", " ".join(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--skip-microc", action="store_true")
    args = parser.parse_args()

    cc = os.environ.get("CC", "gcc")
    llvm_config = shutil.which("llvm-config")
    if not llvm_config:
        print("llvm-config is required", file=sys.stderr)
        return 2

    if not args.skip_microc:
        run(["make", "-B", "microc"])

    micro_sources = [
        "comp/bench_micro_lib.c",
        "src/common.c", "src/instr.c",
        "src/asm/asm386.c",
        "src/codegen/386/codegen386.c",
        "src/codegen/386/lowering.c",
        "src/codegen/386/lowering/call.c",
        "src/codegen/386/lowering/drset.c",
        "src/codegen/386/lowering/fun.c",
        "src/codegen/386/lowering/goto.c",
        "src/codegen/386/lowering/if.c",
        "src/codegen/386/lowering/internal.c",
        "src/codegen/386/lowering/lbl.c",
        "src/codegen/386/lowering/ret.c",
        "src/codegen/386/lowering/set.c",
        "src/codegen/386/lowering/tramp.c",
        "lib/sct/src/arena.c", "lib/sct/src/arena_hashmap.c",
        "lib/sct/src/arena_list.c", "lib/sct/src/arena_vector.c",
        "lib/sct/src/common.c", "lib/sct/src/hashmap.c",
        "lib/sct/src/list.c", "lib/sct/src/string.c",
        "lib/sct/src/vecslice.c", "lib/sct/src/vector.c",
    ]
    common = [cc, "-O2", "-fno-strict-aliasing", "-std=c99", "-Iinclude", "-Ilib/sct/include"]
    run(common + ["-o", str(COMP / "bench_micro_lib"),
                  *micro_sources, "-lm"])
    run([cc, "-m32", "-O2", "-fno-strict-aliasing", "-std=c99", "-Iinclude", "-Ilib/sct/include",
         "-o", str(COMP / "bench_micro_lib32"), *micro_sources, "-lm"])

    run([cc, "-O2", "-std=c99", f"-I{MIR}", "-o", str(COMP / "bench_mir_lib"),
         "comp/bench_mir_lib.c", f"-L{MIR}", "-lmir", "-ldl", "-lm"])
    run([cc, "-O2", "-std=c99", f"-I{MIR}", "-o", str(COMP / "bench_mir_api"),
         "comp/bench_mir_api.c", f"-L{MIR}", "-lmir", "-ldl", "-lm"])

    llvm_cflags = subprocess.check_output([llvm_config, "--cflags"], text=True).split()
    llvm_libs = subprocess.check_output(
        [llvm_config, "--libs", "core", "native", "--ldflags", "--system-libs"],
        text=True,
    ).split()
    run([cc, "-O2", "-std=c99", *llvm_cflags, "-o", str(COMP / "bench_llvm_lib"),
         "comp/bench_llvm_lib.c", *llvm_libs])

    llc = shutil.which("llc")
    if llc:
        with tempfile.TemporaryDirectory(prefix="llvm-i386-") as directory:
            temp = Path(directory)
            workloads = ("fibonacci", "bubblesort", "matrixmul", "gcd", "smoke")
            for workload in workloads:
                source = (COMP / f"{workload}.ll").read_text()
                start = source.find("\ndefine i32 @main()")
                if start >= 0:
                    end = source.find("\n}", start)
                    if end < 0:
                        raise RuntimeError(f"unterminated main in {workload}.ll")
                    source = source[:start] + source[end + 2:]
                (temp / f"{workload}.ll").write_text(source)
            for opt in ("O0", "O2"):
                objects = []
                for workload in workloads:
                    object_path = temp / f"{workload}_{opt}.o"
                    run([llc, f"-{opt}", "-mtriple=i386-unknown-linux-gnu",
                         "-relocation-model=static", "-code-model=small",
                         "-filetype=obj", str(temp / f"{workload}.ll"),
                         "-o", str(object_path)])
                    objects.append(str(object_path))
                run([cc, "-m32", "-O2", "-std=c99", "-fno-pie", "-no-pie", "-o",
                     str(COMP / f"bench_llvm_i386_{opt}"),
                     "comp/bench_llvm_i386.c", *objects])
    else:
        print("llc is absent; i386 LLVM helpers were not built", file=sys.stderr)

    runner_source = ROOT / "micro_runner.c"
    if runner_source.exists():
        run([cc, "-m32", "-O2", "-o", str(ROOT / "micro_runner"), str(runner_source), "-lm"])
    else:
        print("micro_runner.c is absent; API benchmark does not require it", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
