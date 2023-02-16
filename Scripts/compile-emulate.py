#!/usr/bin/env python3

import os, sys

sys.path.append('.')

from Compiler.compilerLib import Compiler

compiler = Compiler()
compiler.prep_compile(build=False)
compiler.execute = True
compiler.options.execute = 'emulate'
compiler.options.ring = compiler.options.ring or '64'
compiler.options.keep_cisc = compiler.options.keep_cisc or ''
compiler.build()
prog = compiler.compile_file()
compiler.local_execution()
