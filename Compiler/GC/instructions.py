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
    XORCI = 0x210,
    BITDECC = 0x211,
    CONVCINT = 0x213,
    REVEAL = 0x214,
    STMSDCI = 0x215,
    INPUTB = 0x216,
    PRINTREGSIGNED = 0x220,
)

class xors(base.Instruction):
    code = opcodes['XORS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class xorm(base.Instruction):
    code = opcodes['XORM']
    arg_format = ['int','sbw','sb','cb']

class xorc(base.Instruction):
    code = base.opcodes['XORC']
    arg_format = ['cbw','cb','cb']

class xorci(base.Instruction):
    code = opcodes['XORCI']
    arg_format = ['cbw','cb','int']

class andrs(base.Instruction):
    code = opcodes['ANDRS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class ands(base.Instruction):
    code = opcodes['ANDS']
    arg_format = tools.cycle(['int','sbw','sb','sb'])

class addc(base.Instruction):
    code = base.opcodes['ADDC']
    arg_format = ['cbw','cb','cb']

class addci(base.Instruction):
    code = base.opcodes['ADDCI']
    arg_format = ['cbw','cb','int']

class mulci(base.Instruction):
    code = base.opcodes['MULCI']
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

class shrci(base.Instruction):
    code = base.opcodes['SHRCI']
    arg_format = ['cbw','cb','int']

class shlci(base.Instruction):
    code = base.opcodes['SHLCI']
    arg_format = ['cbw','cb','int']

class ldbits(base.Instruction):
    code = opcodes['LDBITS']
    arg_format = ['sbw','i','i']

class ldms(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = base.opcodes['LDMS']
    arg_format = ['sbw','int']

class stms(base.DirectMemoryWriteInstruction):
    code = base.opcodes['STMS']
    arg_format = ['sb','int']
    # def __init__(self, *args, **kwargs):
    #     super(type(self), self).__init__(*args, **kwargs)
    #     import inspect
    #     self.caller = [frame[1:] for frame in inspect.stack()[1:]]

class ldmc(base.DirectMemoryInstruction, base.ReadMemoryInstruction):
    code = base.opcodes['LDMC']
    arg_format = ['cbw','int']

class stmc(base.DirectMemoryWriteInstruction):
    code = base.opcodes['STMC']
    arg_format = ['cb','int']

class ldmsi(base.ReadMemoryInstruction):
    code = base.opcodes['LDMSI']
    arg_format = ['sbw','ci']

class stmsi(base.WriteMemoryInstruction):
    code = base.opcodes['STMSI']
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

class movs(base.Instruction):
    code = base.opcodes['MOVS']
    arg_format = ['sbw','sb']

class trans(base.VarArgsInstruction):
    code = opcodes['TRANS']
    def __init__(self, *args):
        self.arg_format = ['int'] + ['sbw'] * args[0] + \
                          ['sb'] * (len(args) - 1 - args[0])
        super(trans, self).__init__(*args)

class bit(base.Instruction):
    code = base.opcodes['BIT']
    arg_format = ['sbw']

class reveal(base.Instruction):
    code = opcodes['REVEAL']
    arg_format = ['int','cbw','sb']

class inputb(base.DoNotEliminateInstruction, base.VarArgsInstruction):
    __slots__ = []
    code = opcodes['INPUTB']
    arg_format = tools.cycle(['p','int','sbw'])

class print_reg(base.IOInstruction):
    code = base.opcodes['PRINTREG']
    arg_format = ['cb','i']
    def __init__(self, reg, comment=''):
        super(print_reg, self).__init__(reg, self.str_to_int(comment))

class print_reg_plain(base.IOInstruction):
    code = base.opcodes['PRINTREGPLAIN']
    arg_format = ['cb']

class print_reg_signed(base.IOInstruction):
    code = opcodes['PRINTREGSIGNED']
    arg_format = ['int','cb']

class print_float_plain(base.IOInstruction):
    __slots__ = []
    code = base.opcodes['PRINTFLOATPLAIN']
    arg_format = ['cb', 'cb', 'cb', 'cb']

class cond_print_str(base.IOInstruction):
    r""" Print a 4 character string. """
    code = base.opcodes['CONDPRINTSTR']
    arg_format = ['cb', 'int']

    def __init__(self, cond, val):
        super(cond_print_str, self).__init__(cond, self.str_to_int(val))
