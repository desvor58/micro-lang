import os
from config import *

if not os.path.isdir(BUILD_DST):
    os.mkdir(BUILD_DST)

for target in BUILD_TARGETS:
    res = os.system(f"{BUILD_COMPILER} {BUILD_COMPILER_FLAGS} -o {BUILD_DST}{target}{BUILD_OUT_EXT} src/{target}.c")

    if res != 0:
        print(f"\033[31mBuilding failed  ({target})\033[0m")
    else:
        print(f"\033[32mBuilding finish successfully  ({target})\033[0m")