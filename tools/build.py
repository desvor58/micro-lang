import os
from config import *

if not os.path.isdir(BUILD_DST):
    os.mkdir(BUILD_DST)

res = os.system(f"{BUILD_COMPILER} {BUILD_COMPILER_FLAGS} -o {BUILD_DST}micro386.exe src/micro386.c")

if res != 0:
    print("\033[31mBuilding failed\033[0m")
else:
    print("\033[32mBuilding finish successfully\033[0m")