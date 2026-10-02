# comp/ — Objective Backend Benchmarks

This directory contains equivalent Fibonacci, bubble-sort, matrix-multiplication, GCD and checksum workloads for micro, LLVM IR and MIR.

| Workload | micro | LLVM IR | MIR |
|---|---|---|---|
| Fibonacci | `fibonacci.micro` | `fibonacci.ll` | `fibonacci.mir` |
| Bubble sort | `bubblesort.micro` | `bubblesort.ll` | `bubblesort.mir` |
| Matrix multiply | `matrixmul.micro` | `matrixmul.ll` | `matrixmul.mir` |
| GCD | `gcd.micro` | `gcd.ll` | `gcd.mir` |
| Smoke | `smoke.micro` | `smoke.ll` | `smoke.mir` |

Every workload name appears in `TESTS` of `benchmark.py`, which `benchmark_i386.py` reuses, and in the workload lists of `build_bench.py` and `dump_llvm_i386.py`. The four helpers each resolve a workload by name in three places: the LLVM, MIR and i386-AOT helpers map a file name to an entry symbol, then pick a call signature, then check the result, and the micro helper builds the program, picks a call signature and checks the result. A new workload has to be added to all of them.

## Smoke test

`smoke.micro`, `smoke.ll` and `smoke.mir` are the checksum workload: it sums `k * w[k]` over an array passed as a pointer, so the result is a closed form number (`n * (n - 1) * (n + 1) / 6`) and does not depend on the host, the timing or the compiler mode. It is deliberately kept inside the limits that any i386 backend has to respect: at most 5 live virtual registers, so nothing spills out of the 6 allocatable ones, `i32` arithmetic only, no floats and no division. That keeps it comparable across the three backends instead of measuring a code path only one of them can take.

It runs as a normal benchmark workload, through `microc`, the LLVM API helper, the MIR API helper and both LLVM i386 AOT levels, with every helper validating the checksum.

`smoke_test.py` adds the checks a timing run cannot make:

```bash
make MODE=debug microc
python3 comp/smoke_test.py
```

It compiles `smoke.micro` with `microc`, runs it through `micro_runner` for 7 input sizes against the closed form, and then inspects the generated listing: the loop must keep every virtual register in a register, the pointer walk must fold into one `lea`, the counter into one `inc`, the multiply into one `imul`, and no division may appear. Any violation prints the full listing and exits non-zero. It needs `micro_runner`, and builds it from `micro_runner.c` when only the source is present.

The pointer walk is written as `+ addr 4` and the counter as `+ i 1` on purpose: those are the shapes the code generator folds into a single instruction.

## Build

Python 3.10 or newer is required.

```bash
python3 comp/build_bench.py
```

The build script creates:

- `comp/bench_micro_lib32`: public micro IR API, x86-32 code generation and in-process execution;
- `comp/bench_micro_lib`: public micro IR API, compile-only 64-bit helper;
- `comp/bench_llvm_lib`: LLVM C API, MCJIT and in-process execution on x86-64;
- `comp/bench_llvm_i386_O0` and `comp/bench_llvm_i386_O2`: LLVM i386 AOT objects linked into 32-bit runners, when `llc` is available;
- `comp/bench_mir_lib`: MIR C API, text-to-MIR plus MIR JIT and in-process execution on x86-64;
- `comp/bench_mir_api`: MIR C API direct IR construction for the VM comparison;
- `micro_runner`, when `micro_runner.c` is available.

The sources are `bench_micro_lib.c`, `bench_llvm_lib.c`, `bench_mir_lib.c`, `bench_mir_api.c` and `bench_llvm_i386.c`; the last one is linked against the `llc` output of every workload in `TESTS`, so its workload list has to match `build_bench.py`.

The LLVM include and link paths are obtained from `llvm-config`; the MIR path is derived from the repository rather than hard-coded to `/home/desvor/dev/micro-lang`.

## Run

```bash
python3 comp/benchmark.py -n 100 -r 3
python3 comp/benchmark.py --hints-only -n 100 -r 3
python3 comp/benchmark.py --no-cli -n 100 -r 3
```

`benchmark.py` writes raw samples and aggregate statistics to `comp/results.json`. A non-zero exit status means that a helper, compilation or correctness check failed.

The old `micro_runner` process is not used as the execution measurement for the API comparison. The API helper performs the equivalent work in-process and reports code allocation/loading separately. A process launch therefore cannot contaminate steady-state execution numbers.

## Results graph

`comp/results.png` is written by every `benchmark.py` run. It answers one question per chart: who is faster, by how much, and in what.

- rows 1 and 2 hold the three headline metrics, compilation, readiness and warmed execution, first as a factor against LLVM and against MIR, then as absolute microseconds;
- row 3 adds the first call, the teardown and the code size micro emits;
- row 4 adds the VM style comparison and the absolute first call.

In a factor chart every bar is one workload against one other backend. The 1.0 line is parity, green bars to the right mean micro wins, red bars to the left mean the other backend wins, and each bar carries its factor as text. The verdict under each chart title counts the wins. Absolute charts keep the backend colours, so a backend looks the same everywhere in the figure.

## VM figures for a presentation

`vm_graphs.py` writes two standalone figures next to `results.png`, both cut from the same VM sample the API benchmark already measures:

- `comp/results_vm_compile.png`: how long one function takes to build through the public IR API, micro against MIR at level 0 and level 2;
- `comp/results_vm_execution.png`: how long the warmed call of that function takes, same three columns.

They are sized for a slide, carry the factor between micro and each MIR level in the header line, and state the ISA caveat in the footer instead of burying it. `benchmark.py` refreshes both on every run; the script also runs standalone against a stored report:

```bash
python3 comp/vm_graphs.py
```

## Measurements

Every helper reports a strict key/value record with requested and successful compilation counts.

- `compile_us`: source/API IR construction through backend code generation;
- `ready_us`: time until a callable native function is available, including executable-memory allocation and copying for micro;
- `load_us`: micro executable-memory allocation and code-copy portion of readiness;
- `first_call_us`: the first invocation after readiness;
- `steady_us`: warmed direct calls through a selected function pointer;
- `cleanup_us`: context and generated-code teardown;
- `vmopt_us`: micro VM metadata-analysis time when hints are enabled;
- `code_size`: emitted micro bytes when available.

The displayed value is the median over `-r` independent helper runs; the table also shows the min/max spread. The number after `-n` is the number of calls in the warmed execution span and the number of independent compilation batches inside one helper run. Bubble-sort and matrix inputs are refilled inside every timed call for all backends, so their workload setup is identical. The script records the platform but does not change CPU frequency or affinity; pin the process externally for publication measurements.

The command-line section is intentionally labelled CLI command latency. It measures process startup, file I/O and the requested compiler command on the checked-in source files, so it is not an equivalent-artifact backend comparison and is not mixed with API/JIT-ready or native execution results. It has two rows, `microc` on the `.micro` source and `clang` on the `.ll` source; MIR has no command line compile path here, because `llc` reads LLVM IR and not MIR text, so the MIR side of that comparison is the API helper.

## VM-style comparison

`--hints-only` compares a small hot function built through both public APIs:

- micro builds the instruction vector through `micro_instr_gen_*`; the hinted variant runs the VM-style liveness pass and supplies `set`/`drset` lifetime metadata before code generation;
- MIR constructs the equivalent loop directly with `MIR_new_func`, `MIR_new_insn`, `MIR_load_module`, `MIR_link` and `MIR_gen`; levels 0, 1, 2 and 3 are reported separately; the local MIR implementation treats level 3 like level 2.

The MIR function returns the same value as the micro `hints` workload (`4950`) and both paths validate the result before timing. No forced branch property is used because the workload has no truthful property to specialize. MIR has no public per-set lifetime field equivalent to micro's `set` hint, so its explicit generator levels are used as the platform-independent optimization metadata.

## i386 LLVM artifacts

The installed LLVM C API is an x86-64 build, so its MCJIT cannot safely execute i386 code inside the existing ELF64 helper. The build script therefore uses `llc -mtriple=i386-unknown-linux-gnu -relocation-model=static -code-model=small` to produce ELF32 objects and links them with `comp/bench_llvm_i386.c` using `-m32`.

`bench_llvm_i386_O0` and `bench_llvm_i386_O2` are AOT execution helpers, not MCJIT helpers; compilation and readiness fields are intentionally unavailable. They validate the same Fibonacci, GCD, bubble-sort and matrix-multiply results as the LLVM API helper. For readable assembly snapshots for all four workloads, run:

```bash
python3 comp/dump_llvm_i386.py
```

This creates `llvm_<workload>_i386_O0` and `llvm_<workload>_i386_O2` using Intel syntax. To run the native i386 comparison after building the helpers:

```bash
python3 comp/benchmark_i386.py -n 100 -r 3
```

It writes `comp/results_i386.json`. The original x86-64 MCJIT benchmark remains separate and must not be described as an i386 API result.

The vendored MIR checkout has no i386 machine backend: its generator dispatch contains x86-64, AArch64, PPC64, S390X and RISC-V targets only. MIR native tests therefore remain explicitly x86-64; compiling MIR with `-m32` or selecting its interpreter would not produce an i386 MIR JIT result.

## Fairness boundaries

micro currently emits raw x86-32 code and its API execution helper is 32-bit. The local LLVM and MIR MCJIT/API helpers are x86-64 and use their native ABIs. LLVM also has separate i386 AOT helpers for native i386 execution and assembly inspection, but the installed LLVM C API cannot run i386 code in the ELF64 MCJIT process. MIR has no i386 backend in this checkout. The default API-ready and steady-state ratios therefore remain `x86-32 micro` versus `x86-64 LLVM/MIR` and are not an ISA-independent code-quality claim; use the i386 LLVM AOT helper only for explicitly labeled i386 comparisons.

The algorithms are semantically equivalent, not guaranteed to be instruction-for-instruction identical; execution comparisons therefore include backend code quality as well as the workload itself.

All API workloads use the same runtime inputs:

- Fibonacci: `n = 30`, result `832040`;
- GCD: `48, 18`, result `6`;
- smoke: 20-element reverse-sorted array, checksum `1330`;
- bubble sort: reverse-sorted 20-element array, full output checked;
- matrix multiply: reverse-sorted 10x10 inputs, every output element checked.

The source files' `main` wrappers are excluded from the algorithm-only API measurements. This prevents workload initialization from being generated and timed for one backend but not another.

## Backend correctness boundary

The benchmark run also exposed backend issues that would make an apparently fast result invalid: unhinted function arguments/labels were given zero lifetime, CLI argument state was uninitialized, and an assembly peephole pass removed live register writes. The benchmark build includes the minimal fixes for those cases; unsafe peephole variants stay disabled until register liveness is modeled correctly.

## Standalone helpers

```bash
./comp/bench_micro_lib32 fibonacci 100 plain
./comp/bench_micro_lib32 hints 100 hinted
./comp/bench_llvm_lib comp/fibonacci.ll 100
./comp/bench_llvm_i386_O2 comp/bubblesort 100 O2
./comp/bench_mir_lib comp/fibonacci.mir 100 2
./comp/bench_mir_api 100 2
```

The optional micro mode `hinted` applies the liveness metadata only to the dedicated `hints` workload. The larger control-flow workloads remain on the conservative unhinted path because the current backend lifetime semantics do not yet provide a CFG-safe lifetime for every loop-carried value.

## Assembly snapshots

`bubblesort_hinted.micro` is the source used for `micro_bubblesort`. It uses full prefix expressions directly in loop conditions and address calculations, and applies finite lifetime hints to temporaries whose last use is in the same straight-line region while keeping loop-carried values live. The API benchmark's bubble-sort builder mirrors this expression form. The swap path keeps named address/value temporaries because `drset` requires a named pointer destination. `micro_bubblesort` is the internal x86-32 micro-asm listing emitted by `microc -Pa`; `micro_bubblesort_i386` is the same raw code shown as Intel-syntax x86-32 disassembly:

```bash
./bin/microc -Pa -o /tmp/bubblesort.bin comp/bubblesort_hinted.micro > comp/micro_bubblesort
./bin/microc -o /tmp/bubblesort.bin comp/bubblesort_hinted.micro
objdump -D -b binary -m i386 -M intel /tmp/bubblesort.bin \
    | sed -n '/^00000000 <\\.data>:/,$p' \
    | sed '1s/.*/micro_bubblesort:/' > comp/micro_bubblesort_i386
```

`mir_bubblesort_O0` and `mir_bubblesort_O2` contain the x86-64 function generated from the algorithm-only `bubblesort.mir` module at MIR optimization levels 0 and 2. They are Intel-syntax disassemblies with addresses normalized to zero; the `main` wrapper is not included.
