#!/usr/bin/env python3
import argparse
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
COMP = ROOT / "comp"
TESTS = ("fibonacci", "bubblesort", "matrixmul", "gcd", "smoke")


def algorithm_source(source: str) -> str:
    start = source.find("\ndefine i32 @main()")
    if start < 0:
        return source
    end = source.find("\n}", start)
    if end < 0:
        raise ValueError("unterminated main function")
    return source[:start] + source[end + 2:]


def clean_asm(source: str) -> str:
    lines = []
    for line in source.splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#") or stripped.startswith(".cfi_"):
            continue
        if " #" in line:
            line = line.split(" #", 1)[0].rstrip()
        lines.append(line.rstrip())
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--llc", default=shutil.which("llc") or "llc")
    parser.add_argument("--level", type=int, choices=(0, 2), action="append")
    args = parser.parse_args()
    levels = args.level or [0, 2]
    llc = shutil.which(args.llc) or args.llc
    if not Path(llc).exists() and not shutil.which(llc):
        raise SystemExit(f"llc not found: {args.llc}")

    with tempfile.TemporaryDirectory(prefix="llvm-i386-") as directory:
        temp = Path(directory)
        for workload in TESTS:
            source_path = COMP / f"{workload}.ll"
            source = temp / f"{workload}.ll"
            source.write_text(algorithm_source(source_path.read_text()))
            for level in levels:
                output = COMP / f"llvm_{workload}_i386_O{level}"
                assembly = temp / f"{workload}_O{level}.s"
                subprocess.run([
                    llc,
                    f"-O{level}",
                    "-mtriple=i386-unknown-linux-gnu",
                    "-relocation-model=static",
                    "-code-model=small",
                    "-x86-asm-syntax=intel",
                    "-filetype=asm",
                    str(source),
                    "-o",
                    str(assembly),
                ], check=True)
                output.write_text(clean_asm(assembly.read_text()))
                print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
