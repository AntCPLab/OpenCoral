#!/usr/bin/env python3


#     ===== Compiler usage instructions =====
# 
# ./compile.py input_file
# 
# will compile Programs/Source/input_file.asm onto
# Programs/Bytecode/input_file.bc
# 
# (run with --help for more options)
# 
# See Compiler/README for details on the Compiler package


from optparse import OptionParser
import Compiler

def main():
    usage = "usage: %prog [options] filename [args]"
    parser = OptionParser(usage=usage)
    parser.add_option("-n", "--nomerge",
                      action="store_false", dest="merge_opens", default=True,
                      help="don't attempt to merge open instructions")
    parser.add_option("-o", "--output", dest="outfile",
                      help="specify output file")
    parser.add_option("-a", "--asm-output", dest="asmoutfile",
                      help="asm output file for debugging")
    parser.add_option("-p", "--primesize", dest="param", default=-1,
                      help="bit length of modulus")
    parser.add_option("-g", "--galoissize", dest="galois", default=40,
                      help="bit length of Galois field")
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                      help="keep track of trace for debugging")
    parser.add_option("-c", "--comparison", dest="comparison", default="log",
                      help="comparison variant: log|plain|inv|sinv")
    parser.add_option("-r", "--noreorder", dest="reorder_between_opens",
                      action="store_false", default=True,
                      help="don't attempt to place instructions between start/stop opens")
    parser.add_option("-M", "--preserve-mem-order", action="store_true",
                      dest="preserve_mem_order", default=False,
                      help="preserve order of memory instructions; possible efficiency loss")
    parser.add_option("-O", "--optimize-hard", action="store_true",
                      dest="optimize_hard", help="currently not in use")
    parser.add_option("-u", "--noreallocate", action="store_true", dest="noreallocate",
                      default=False, help="don't reallocate")
    parser.add_option("-m", "--max-parallel-open", dest="max_parallel_open",
                      default=False, help="restrict number of parallel opens")
    parser.add_option("-D", "--dead-code-elimination", action="store_true",
                      dest="dead_code_elimination", default=False,
                      help="eliminate instructions with unused result")
    parser.add_option("-P", "--profile", action="store_true", dest="profile",
                      help="profile compilation")
    parser.add_option("-s", "--stop", action="store_true", dest="stop",
                      help="stop on register errors")
    parser.add_option("-R", "--ring", dest="ring", default=0,
                      help="bit length of ring (default: 0 for field)")
    parser.add_option("-B", "--binary", dest="binary", default=0,
                      help="bit length of sint in binary circuit (default: 0 for arithmetic)")
    parser.add_option("-F", "--field", dest="field", default=0,
                      help="bit length of sint modulo prime (default: 64)")
    parser.add_option("-I", "--insecure", action="store_true", dest="insecure",
                      help="activate insecure functionality for benchmarking")
    parser.add_option("-b", "--budget", dest="budget", default=100000,
                      help="set budget for optimized loop unrolling "
                      "(default: 100000)")
    parser.add_option("-X", "--mixed", action="store_true", dest="mixed",
                      help="mixing arithmetic and binary computation")
    parser.add_option("-Y", "--edabit", action="store_true", dest="edabit",
                      help="mixing arithmetic and binary computation using edaBits")
    parser.add_option("-C", "--CISC", action="store_true", dest="cisc",
                      help="faster CISC compilation mode")
    parser.add_option("-v", "--verbose", action="store_true", dest="verbose",
                      help="more verbose output")
    options,args = parser.parse_args()
    if len(args) < 1:
        parser.print_help()
        return

    if options.optimize_hard:
        print('Note that -O/--optimize-hard currently has no effect')

    def compilation():
        prog = Compiler.run(args, options, param=int(options.param),
                            merge_opens=options.merge_opens,
                            debug=options.debug)
        prog.write_bytes(options.outfile)

        if options.asmoutfile:
            for tape in prog.tapes:
                tape.write_str(options.asmoutfile + '-' + tape.name)

        if prog.public_input_file is not None:
            print('WARNING: %s is required to run the program' % \
                  prog.public_input_file.name)

    if options.profile:
        import cProfile
        p = cProfile.Profile().runctx('compilation()', globals(), locals())
        p.dump_stats(args[0] + '.prof')
        p.print_stats(2)
    else:
        compilation()

if __name__ == '__main__':
    main()
