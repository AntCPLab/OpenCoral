#!/usr/bin/env python3

import sys, os
import collections

sys.path.append('.')

from Compiler.program import *
from Compiler.instructions_base import *

if len(sys.argv) <= 1:
    print('Usage: %s <program>' % sys.argv[0])

res = collections.defaultdict(lambda: 0)
m = 0

for tapename in Program.read_tapes(sys.argv[1]):
    for inst in Tape.read_instructions(tapename):
        t = inst.type
        if issubclass(t, DirectMemoryInstruction):
            res[t.arg_format[0]] = max(inst.args[1].i + inst.size,
                                       res[t.arg_format[0]])
        for arg in inst.args:
            if isinstance(arg, RegisterArgFormat):
                m = max(m, arg.i + inst.size)

print (res)
print (m)

