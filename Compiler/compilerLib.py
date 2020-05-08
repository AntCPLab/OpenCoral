from Compiler.program import Program
from Compiler.config import *
from Compiler.exceptions import *
from . import instructions, instructions_base, types, comparison, library
from .GC import types as GC_types

import random
import time
import sys


def run(args, options, param=-1, merge_opens=True,
            reallocate=True, debug=False):
    """ Compile a file and output a Program object.
    
    If merge_opens is set to True, will attempt to merge any parallelisable open
    instructions. """
    
    prog = Program(args, options, param)
    instructions.program = prog
    instructions_base.program = prog
    types.program = prog
    comparison.program = prog
    prog.DEBUG = debug
    VARS['program'] = prog
    if options.binary:
        VARS['sint'] = GC_types.sbitint.get_type(int(options.binary))
        VARS['sfix'] = GC_types.sbitfix
    comparison.set_variant(options)
    
    print('Compiling file', prog.infile)
    
    if instructions_base.Instruction.count != 0:
        print('instructions count', instructions_base.Instruction.count)
        instructions_base.Instruction.count = 0
    # make compiler modules directly accessible
    sys.path.insert(0, 'Compiler')
    # create the tapes
    exec(compile(open(prog.infile).read(), prog.infile, 'exec'), VARS)
    
    # optimize the tapes
    for tape in prog.tapes:
        tape.optimize(options)
    
    if prog.tapes:
        prog.update_req(prog.curr_tape)

    if prog.req_num:
        print('Program requires:')
        for x in prog.req_num.pretty():
            print(x)

    if prog.verbose:
        print('Program requires:', repr(prog.req_num))
        print('Cost:', 0 if prog.req_num is None else prog.req_num.cost())
        print('Memory size:', dict(prog.allocated_mem))

    # finalize the memory
    prog.finalize_memory()

    return prog
