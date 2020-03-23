# This script calculates how many VM opcodes we currently have, i use this to ensure i don't hit the limit for a byte.
import sys

count = 0;

path = "src/opcodes.h"
# So i can call it from anywhere e.g python ../scripts/opcodes.py opcodes.h from src/ without changing dir to root.
if len(sys.argv) > 1:
    path = sys.argv[1]

with open(path) as f:
    f = f.readlines()
    for x in f:
        if "PVM_OP_" in x:
            count += 1


print(f"There is currently {count} VM opcodes.")
if count > 256:
    print(f"WARNING: opcodes is over the limit of a byte ")
