import Compiler.instructions_base as base
import Compiler.instructions as spdz
import Compiler.tools as tools
import collections
import itertools

class SecretBitsAF(base.RegisterArgFormat):
    reg_type = 'sb'
class ClearBitsAF(base.RegisterArgFormat):
    reg_type = 'cb'

base.ArgFormats['sb'] = SecretBitsAF
base.ArgFormats['sbw'] = SecretBitsAF
base.ArgFormats['cb'] = ClearBitsAF
base.ArgFormats['cbw'] = ClearBitsAF

opcodes = dict(
    XORS = 0x200,
    XORM = 0x201,
    ANDRS = 0x202,
    BITDECS = 0x203,
    BITCOMS = 0x204,
    CONVSINT = 0x205,
    LDMSDI = 0x206,
    STMSDI = 0x207,
    LDMSD = 0x208,
    STMSD = 0x209,
    LDBITS = 0x20a,
    ANDS = 0x20b,
    TRANS = 0x20c,
    BITB = 0x20d,
    ANDM = 0x20e,
    NOTS = 0x20f,
    LDMSB = 0x240,
    STMSB = 0x241,
    LDMSBI = 0x242,
    STMSBI = 0x243,
    MOVSB = 0x244,
    INPUTB = 0x246,
    INPUTBVEC = 0x247,
    SPLIT = 0x248,
    CONVCBIT2S = 0x249,
    XORCBI = 0x210,
    BITDECC = 0x211,
    CONVCINT = 0x213,
    REVEAL = 0x214,
    STMSDCI = 0x215,
    LDMCB = 0x217,
    STMCB = 0x218,
    XORCB = 0x219,
    ADDCB = 0x21a,
    ADDCBI = 0x21b,
    MULCBI = 0x21c,
    SHRCBI = 0x21d,
    SHLCBI = 0x21e,
    CONVCINTVEC = 0x21f,
    PRINTREGSIGNED = 0x220,
    PRINTREGB = 0x221,
    PRINTREGPLAINB = 0x222,
    PRINTFLOATPLAINB = 0x223,
    CONDPRINTSTRB = 0x224,
    CONVCBIT = 0x230,
    CONVCBITVEC = 0x231,
)

class BinaryVectorInstruction(base.Instruction):
    is_vec = lambda self: True

    def copy(self, size, subs):
        return type(self)(*self.get_new_args(size, subs))

class NonVectorInstruction(base.Instruction):
    is_vec = lambda self: False

    def __init__(self, *args, **kwargs):
        assert(args[0].n <= args[0].unit)
        super(NonVectorInstruction, self).__init__(*args, **kwargs)

class NonVectorInstruction1(base.Instruction):
    is_vec = lambda self: False

    def __init__(self, *args, **kwargs):
        assert(args[1].n <= args[1].unit)
        super(NonVectorInstruction1, self).__init__(*args, **kwargs)

class xors(BinaryVectorInstruction):
    code = opcodes['XORS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class xorm(NonVectorInstruction):
    code = opcodes['XORM']
    arg_format = ['int','sbw','sb','cb']

class xorcb(NonVectorInstruction):
    code = opcodes['XORCB']
    arg_format = ['cbw','cb','cb']

class xorcbi(NonVectorInstruction):
    code = opcodes['XORCBI']
    arg_format = ['cbw','cb','int']

class andrs(BinaryVectorInstruction):
    code = opcodes['ANDRS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

    def add_usage(self, req_node):
        req_node.increment(('bit', 'triple'), sum(self.args[::4]))

class ands(BinaryVectorInstruction):
    code = opcodes['ANDS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

    def add_usage(self, req_node):
        req_node.increment(('bit', 'triple'), sum(self.args[::4]))

class andm(BinaryVectorInstruction):
    code = opcodes['ANDM']
    arg_format = ['int','sbw','sb','cb']

class nots(BinaryVectorInstruction):
    code = opcodes['NOTS']
    arg_format = ['int','sbw','sb']

class addcb(NonVectorInstruction):
    code = opcodes['ADDCB']
    arg_format = ['cbw','cb','cb']

class addcbi(NonVectorInstruction):
    code = opcodes['ADDCBI']
    arg_format = ['cbw','cb','int']

class mulcbi(NonVectorInstruction):
    code = opcodes['MULCBI']
    arg_format = ['cbw','cb','int']

class bitdecs(NonVectorInstruction, base.VarArgsInstruction):
    code = opcodes['BITDECS']
    arg_format = tools.chain(['sb'], itertools.repeat('sbw'))

class bitcoms(NonVectorInstruction, base.VarArgsInstruction):
    code = opcodes['BITCOMS']
    arg_format = tools.chain(['sbw'], itertools.repeat('sb'))

class bitdecc(NonVectorInstruction, base.VarArgsInstruction):
    code = opcodes['BITDECC']
    arg_format = tools.chain(['cb'], itertools.repeat('cbw'))

class shrcbi(NonVectorInstruction):
    code = opcodes['SHRCBI']
    arg_format = ['cbw','cb','int']

class shlcbi(NonVectorInstruction):
    code = opcodes['SHLCBI']
    arg_format = ['cbw','cb','int']

class ldbits(NonVectorInstruction):
    code = opcodes['LDBITS']
    arg_format = ['sbw','i','i']

class ldmsb(base.DirectMemoryInstruction, base.ReadMemoryInstruction,
            base.VectorInstruction):
    code = opcodes['LDMSB']
    arg_format = ['sbw','int']

class stmsb(base.DirectMemoryWriteInstruction, base.VectorInstruction):
    code = opcodes['STMSB']
    arg_format = ['sb','int']
    # def __init__(self, *args, **kwargs):
    #     super(type(self), self).__init__(*args, **kwargs)
    #     import inspect
    #     self.caller = [frame[1:] for frame in inspect.stack()[1:]]

class ldmcb(base.DirectMemoryInstruction, base.ReadMemoryInstruction,
            base.VectorInstruction):
    code = opcodes['LDMCB']
    arg_format = ['cbw','int']

class stmcb(base.DirectMemoryWriteInstruction, base.VectorInstruction):
    code = opcodes['STMCB']
    arg_format = ['cb','int']

class ldmsbi(base.ReadMemoryInstruction, base.VectorInstruction):
    code = opcodes['LDMSBI']
    arg_format = ['sbw','ci']

class stmsbi(base.WriteMemoryInstruction, base.VectorInstruction):
    code = opcodes['STMSBI']
    arg_format = ['sb','ci']

class ldmsdi(base.ReadMemoryInstruction):
    code = opcodes['LDMSDI']
    arg_format = tools.cycle(['sbw','cb','int'])

class stmsdi(base.WriteMemoryInstruction):
    code = opcodes['STMSDI']
    arg_format = tools.cycle(['sb','cb'])

class ldmsd(base.ReadMemoryInstruction):
    code = opcodes['LDMSD']
    arg_format = tools.cycle(['sbw','int','int'])

class stmsd(base.WriteMemoryInstruction):
    code = opcodes['STMSD']
    arg_format = tools.cycle(['sb','int'])

class stmsdci(base.WriteMemoryInstruction):
    code = opcodes['STMSDCI']
    arg_format = tools.cycle(['cb','cb'])

class convsint(NonVectorInstruction1):
    code = opcodes['CONVSINT']
    arg_format = ['int','sbw','ci']

class convcint(NonVectorInstruction):
    code = opcodes['CONVCINT']
    arg_format = ['cbw','ci']

class convcbit(NonVectorInstruction1):
    code = opcodes['CONVCBIT']
    arg_format = ['ciw','cb']

@base.vectorize
class convcintvec(base.Instruction):
    code = opcodes['CONVCINTVEC']
    arg_format = tools.chain(['c'], tools.cycle(['cbw']))

class convcbitvec(BinaryVectorInstruction):
    code = opcodes['CONVCBITVEC']
    arg_format = ['int','ciw','cb']
    def __init__(self, *args):
        super(convcbitvec, self).__init__(*args)
        assert(args[2].n == args[0])
        args[1].set_size(args[0])

class convcbit2s(BinaryVectorInstruction):
    code = opcodes['CONVCBIT2S']
    arg_format = ['int','sbw','cb']

@base.vectorize
class split(base.Instruction):
    code = opcodes['SPLIT']
    arg_format = tools.chain(['int','s'], tools.cycle(['sbw']))
    def __init__(self, *args, **kwargs):
        super(split_class, self).__init__(*args, **kwargs)
        assert (len(args) - 2) % args[0] == 0

class movsb(NonVectorInstruction):
    code = opcodes['MOVSB']
    arg_format = ['sbw','sb']

class trans(base.VarArgsInstruction):
    code = opcodes['TRANS']
    is_vec = lambda self: True
    def __init__(self, *args):
        self.arg_format = ['int'] + ['sbw'] * args[0] + \
                          ['sb'] * (len(args) - 1 - args[0])
        super(trans, self).__init__(*args)

class bitb(NonVectorInstruction):
    code = opcodes['BITB']
    arg_format = ['sbw']

class reveal(BinaryVectorInstruction, base.VarArgsInstruction, base.Mergeable):
    code = opcodes['REVEAL']
    arg_format = tools.cycle(['int','cbw','sb'])

class inputb(base.DoNotEliminateInstruction, base.VarArgsInstruction):
    __slots__ = []
    code = opcodes['INPUTB']
    arg_format = tools.cycle(['p','int','int','sbw'])
    is_vec = lambda self: True

class inputbvec(base.DoNotEliminateInstruction, base.VarArgsInstruction,
                base.Mergeable):
    __slots__ = []
    code = opcodes['INPUTBVEC']

    def __init__(self, *args, **kwargs):
        self.arg_format = []
        i = 0
        while i < len(args):
            self.arg_format += ['int', 'int', 'p'] + ['sbw'] * (args[i]  - 3)
            i += args[i]
        assert i == len(args)
        super(inputbvec, self).__init__(*args, **kwargs)

    def merge(self, other):
        self.args += other.args
        self.arg_format += other.arg_format

class print_regb(base.VectorInstruction, base.IOInstruction):
    code = opcodes['PRINTREGB']
    arg_format = ['cb','i']
    def __init__(self, reg, comment=''):
        super(print_regb, self).__init__(reg, self.str_to_int(comment))

class print_reg_plainb(NonVectorInstruction, base.IOInstruction):
    code = opcodes['PRINTREGPLAINB']
    arg_format = ['cb']

class print_reg_signed(base.IOInstruction):
    code = opcodes['PRINTREGSIGNED']
    arg_format = ['int','cb']
    is_vec = lambda self: True

class print_float_plainb(base.IOInstruction):
    __slots__ = []
    code = opcodes['PRINTFLOATPLAINB']
    arg_format = ['cb', 'cb', 'cb', 'cb', 'cb']

class cond_print_strb(base.IOInstruction):
    r""" Print a 4 character string. """
    code = opcodes['CONDPRINTSTRB']
    arg_format = ['cb', 'int']

    def __init__(self, cond, val):
        super(cond_print_str, self).__init__(cond, self.str_to_int(val))
