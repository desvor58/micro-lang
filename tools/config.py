import sys

BUILD_COMPILER       = "gcc"  # compiler for compiling scc
BUILD_COMPILER_FLAGS = "-std=c99 -Iinclude"  # options for compiler, witch compile scc
BUILD_DST            = "./bin/"  # where to build scc
BUILD_OUT_EXT        = ".exe" if sys.platform == "win32" else ""

BUILD_TARGETS = [
    "micro-386"
]