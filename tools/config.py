import sys

BUILD_COMPILER       = "gcc"  # compiler for compiling micro  clang not working)))
BUILD_COMPILER_FLAGS = "-std=c99 -Llib -lsct -Iinclude -Wall -Wno-missing-braces -Wno-format"  # options for compiler, witch compile micro
BUILD_DST            = "./bin/"  # where to build micro
BUILD_OUT_EXT        = ".exe" if sys.platform == "win32" else ""
BUILD_OUT            = "micro"
BUILD_TARGETS = [
    "src/c/*.c",
    "src/codegen/386/c/*.c",
    "src/codegen/386/c/statements/*.c",
    "src/codegen/386/c/expr_get_atoms/*.c",
    "src/codegen/c/*.c",
    "src/micro.c",
]