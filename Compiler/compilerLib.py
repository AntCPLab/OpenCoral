from Compiler.program import Program
from .GC import types as GC_types

import sys


def run(args, options):
    """ Compile a file and output a Program object.
    
    If options.merge_opens is set to True, will attempt to merge any
    parallelisable open instructions. """
    
    prog = Program(args, options)
    VARS['program'] = prog
    if options.binary:
        VARS['sint'] = GC_types.sbitintvec.get_type(int(options.binary))
        VARS['sfix'] = GC_types.sbitfixvec
        for i in 'cint', 'cfix', 'cgf2n', 'sintbit', 'sgf2n', 'sgf2nint', \
            'sgf2nuint', 'sgf2nuint32', 'sgf2nfloat', 'sfloat', 'cfloat', \
            'squant':
            del VARS[i]
    
    print('Compiling file', prog.infile)
    
    # make compiler modules directly accessible
    sys.path.insert(0, 'Compiler')
    # create the tapes
    exec(compile(open(prog.infile).read(), prog.infile, 'exec'), VARS)

    prog.finalize()

    if prog.req_num:
        print('Program requires:')
        for x in prog.req_num.pretty():
            print(x)

    if prog.verbose:
        print('Program requires:', repr(prog.req_num))
        print('Cost:', 0 if prog.req_num is None else prog.req_num.cost())
        print('Memory size:', dict(prog.allocated_mem))

    return prog
