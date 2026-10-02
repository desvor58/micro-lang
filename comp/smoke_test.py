#!/usr/bin/env python3
#
# smoke_test.py — architecture neutral smoke test
#
# Compiles comp/smoke.micro, runs it and checks the result against a
# closed form. On top of the numbers it checks that the generated code
# stays inside the limits every i386 backend has to respect: only the 6
# allocatable registers, integer types, and no virtual register spilled
# to the frame inside the loop.
#
# usage: python3 comp/smoke_test.py

import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
MICROC = ROOT / "bin" / "microc"
SOURCE = ROOT / "comp" / "smoke.micro"
RUNNER = ROOT / "micro_runner"
RUNNER_SRC = ROOT / "micro_runner.c"

SIZES = (1, 2, 3, 7, 16, 64, 257)
ALLOC_REGS = ("eax", "ecx", "edx", "ebx", "esi", "edi")


def die(msg):
    print(f"smoke_test: {msg}", file=sys.stderr)
    sys.exit(1)


def build_runner():
    if not RUNNER.exists():
        if RUNNER_SRC.exists():
            run(["gcc", "-m32", "-o", str(RUNNER), str(RUNNER_SRC)])
            print(f"smoke_test: built {RUNNER.name}")
        else:
            die(f"no runner found, build it with 'gcc -m32 -o micro_runner micro_runner.c'")


def run(cmd):
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0 or res.stdout or res.stderr:
        die(f"{' '.join(str(c) for c in cmd)} failed\n{res.stdout}{res.stderr}")


def compile_source(binary):
    res = subprocess.run([MICROC, "-Pa", "-o", binary, SOURCE], capture_output=True, text=True)
    if res.returncode != 0 or res.stderr or "Error" in res.stdout:
        die(f"compilation failed\n{res.stdout}{res.stderr}")
    return res.stdout


def check_result(binary, n):
    out = subprocess.run([str(RUNNER), str(binary), "1", f"@{4 * n}", str(n)],
                         capture_output=True, text=True)
    if out.returncode != 0:
        die(f"runner exited with {out.returncode} for n = {n}\n{out.stdout}{out.stderr}")
    got = re.findall(r"^\s*result\s*=\s*(-?\d+)", out.stdout, re.M)
    if not got:
        die(f"no result for n = {n}\n{out.stdout}")
    return int(got[-1])


def reference(n):
    return n * (n - 1) * (n + 1) // 6


def check_listing(listing):
    body = listing.split("checksum.loop:", 1)[1].split("checksum.done:", 1)
    if len(body) != 2:
        die("loop or done label is missing in the listing")

    loop, done = body
    checks = (
        ("registers", len(set(re.findall(r"\b" + "|".join(ALLOC_REGS) + r"\b", listing))) <= 6),
        ("loop keeps vregs in registers", not re.search(r"(movS32R32|movR32S32|leaR32SIBABS)", loop)),
        ("pointer walk is one lea", "leaR32SIBI8" in loop),
        ("counter is one inc", "incR32" in loop),
        ("multiply is one imul", "imulR32" in loop),
        ("no division", not re.search(r"\b(idiv|div)", listing)),
        ("return value is one slot", len(re.findall(r"-4\b", done)) == 2),
    )
    for name, ok in checks:
        print(f"  [{'ok' if ok else 'FAIL'}] {name}")
        if not ok:
            print(listing)
            sys.exit(1)


def main():
    if not MICROC.exists():
        die(f"no compiler found, build it with 'make MODE=debug microc'")
    build_runner()

    binary = pathlib.Path("/tmp/micro_smoke_test.bin")
    print("smoke_test: compiling smoke.micro")
    listing = compile_source(binary)
    print(f"smoke_test: {binary.stat().st_size} bytes of code")

    print("smoke_test: code shape")
    check_listing(listing)

    print("smoke_test: results")
    for n in SIZES:
        got = check_result(binary, n)
        want = reference(n)
        ok = got == want
        print(f"  [{'ok' if ok else 'FAIL'}] n = {n:4d}  checksum = {got:12d}  expected {want:12d}")
        if not ok:
            sys.exit(1)

    print("smoke_test: passed")


if __name__ == "__main__":
    main()