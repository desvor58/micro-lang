import os
from config import *

if not os.path.isdir(BUILD_DST):
    os.mkdir(BUILD_DST)

res = os.system(f"{BUILD_COMPILER} -o {BUILD_DST}{BUILD_OUT}{BUILD_OUT_EXT} {" ".join(BUILD_TARGETS)} {BUILD_COMPILER_FLAGS}")

if res != 0:
    print(f"\033[31mBuilding failed\033[0m")
else:
    print(f"\033[32mBuilding finish successfully\033[0m")