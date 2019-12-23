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
    LDMSB = 0x240,
    STMSB = 0x241,
    LDMSBI = 0x242,
    STMSBI = 0x243,
    MOVSB = 0x244,
    INPUTB = 0x246,
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
    PRINTREGSIGNED = 0x220,
    PRINTREGB = 0x221,
    PRINTREGPLAINB = 0x222,
    PRINTFLOATPLAINB = 0x223,
    CONDPRINTSTRB = 0x224,
    CONVCBIT = 0x230,
)

class xors(base.Instruction):
    code = opcodes['XORS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class xorm(base.Instruction):
    code = opcodes['XORM']
    arg_format = ['int','sbw','sb','cb']

class xorcb(base.Instruction):
    code = opcodes['XORCB']
    arg_format = ['cbw','cb','cb']

class xorcbi(base.Instruction):
    code = opcodes['XORCBI']
    arg_format = ['cbw','cb','int']

class andrs(base.Instruction):
    code = opcodes['ANDRS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class ands(base.Instruction):
    code = opcodes['ANDS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class andm(base.Instruction):
    code = opcodes['ANDM']
    arg_format = ['int','sbw','sb','cb']

class addcb(base.Instruction):
    code = opcodes['ADDCB']
    arg_format = ['cbw','cb','cb']

class addcbi(base.Instruction):
    code = opcodes['ADDCBI']
    arg_format = ['cbw','cb','int']

class mulcbi(base.Instruction):
    code = opcodes['MULCBI']
    arg_format = ['cbw','cb','int']

class bitdecs(base.VarArgsInstruction):
    code = opcodes['BITDECS']
    arg_format = tools.chain(['sb'], itertools.repeat('sbw'))

class bitcoms(base.VarArgsInstruction):
    code = opcodes['BITCOMS']
    arg_format = tools.chain(['sbw'], itertools.repeat('sb'))

class bitdecc(base.VarArgsInstruction):
    code = opcodes['BITDECC']
    arg_format = tools.chain(['cb'], itertools.repeat('cbw'))

class shrcbi(base.Instruction):
    code = opcodes['SHRCBI']
    arg_format = ['cbw','cb','int']

class shlcbi(base.Instruction):
    code = opcodes['SHLCBI']
    arg_format = ['cbw','cb','int']

class ldbits(base.Instruction):
    code = opcodes['LDBITS']
    arg_format = ['sbw','i','i']

class ldmsb(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = opcodes['LDMSB']
    arg_format = ['sbw','int']

class stmsb(base.DirectMemoryWriteInstruction):
    code = opcodes['STMSB']
    arg_format = ['sb','int']
    # def __init__(self, *args, **kwargs):
    #     super(type(self), self).__init__(*args, **kwargs)
    #     import inspect
    #     self.caller = [frame[1:] for frame in inspect.stack()[1:]]

class ldmcb(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = opcodes['LDMCB']
    arg_format = ['cbw','int']

class stmcb(base.DirectMemoryWriteInstruction):
    code = opcodes['STMCB']
    arg_format = ['cb','int']

class ldmsbi(base.ReadMemoryInstruction):
    code = opcodes['LDMSBI']
    arg_format = ['sbw','ci']

class stmsbi(base.WriteMemoryInstruction):
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

class convsint(base.Instruction):
    code = opcodes['CONVSINT']
    arg_format = ['int','sbw','ci']

class convcint(base.Instruction):
    code = opcodes['CONVCINT']
    arg_format = ['cbw','ci']

class convcbit(base.Instruction):
    code = opcodes['CONVCBIT']
    arg_format = ['ciw','cb']

class movsb(base.Instruction):
    code = opcodes['MOVSB']
    arg_format = ['sbw','sb']

class trans(base.VarArgsInstruction):
    code = opcodes['TRANS']
    def __init__(self, *args):
        self.arg_format = ['int'] + ['sbw'] * args[0] + \
                          ['sb'] * (len(args) - 1 - args[0])
        super(trans, self).__init__(*args)

class bitb(base.Instruction):
    code = opcodes['BITB']
    arg_format = ['sbw']

class reveal(base.Instruction):
    code = opcodes['REVEAL']
    arg_format = ['int','cbw','sb']

class inputb(base.DoNotEliminateInstruction, base.VarArgsInstruction):
    __slots__ = []
    code = opcodes['INPUTB']
    arg_format = tools.cycle(['p','int','int','sbw'])

class print_regb(base.IOInstruction):
    code = opcodes['PRINTREGB']
    arg_format = ['cb','i']
    def __init__(self, reg, comment=''):
        super(print_regb, self).__init__(reg, self.str_to_int(comment))

class print_reg_plainb(base.IOInstruction):
    code = opcodes['PRINTREGPLAINB']
    arg_format = ['cb']

class print_reg_signed(base.IOInstruction):
    code = opcodes['PRINTREGSIGNED']
    arg_format = ['int','cb']

class print_float_plainb(base.IOInstruction):
    __slots__ = []
    code = opcodes['PRINTFLOATPLAINB']
    arg_format = ['cb', 'cb', 'cb', 'cb']

class cond_print_strb(base.IOInstruction):
    r""" Print a 4 character string. """
    code = opcodes['CONDPRINTSTRB']
    arg_format = ['cb', 'int']

    def __init__(self, cond, val):
        super(cond_print_str, self).__init__(cond, self.str_to_int(val))
