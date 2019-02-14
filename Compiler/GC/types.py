from Compiler.types import MemValue, read_mem_value, regint, Array
from Compiler.types import _bitint, _number, _fix, _structure
from Compiler.program import Tape, Program
from Compiler.exceptions import *
from Compiler import util, oram, floatingpoint
import Compiler.GC.instructions as inst
import operator

class bits(Tape.Register, _structure):
    n = 40
    size = 1
    PreOp = staticmethod(floatingpoint.PreOpN)
    decomposed = None
    @staticmethod
    def PreOR(l):
        return [1 - x for x in \
                floatingpoint.PreOpN(operator.mul, \
                                     [1 - x for x in l])]
    @classmethod
    def get_type(cls, length):
        if length is None:
            return cls
        elif length == 1:
            return cls.bit_type
        if length not in cls.types:
            class bitsn(cls):
                n = length
            cls.types[length] = bitsn
            bitsn.__name__ = cls.__name__ + str(length)
        return cls.types[length]
    @classmethod
    def conv(cls, other):
        if isinstance(other, cls):
            return other
        elif isinstance(other, MemValue):
            return cls.conv(other.read())
        else:
            res = cls()
            res.load_other(other)
            return res
    hard_conv = conv
    @classmethod
    def compose(cls, items, bit_length=1):
        return cls.bit_compose(sum([util.bit_decompose(item, bit_length) for item in items], []))
    @classmethod
    def bit_compose(cls, bits):
        if len(bits) == 1:
            return bits[0]
        bits = list(bits)
        res = cls.new(n=len(bits))
        cls.bitcom(res, *(sbit.conv(bit) for bit in bits))
        res.decomposed = bits
        return res
    def bit_decompose(self, bit_length=None):
        n = bit_length or self.n
        suffix = [0] * (n - self.n)
        if n == 1 and self.n == 1:
            return [self]
        n = min(n, self.n)
        if self.decomposed is None or len(self.decomposed) < n:
            res = [self.bit_type() for i in range(n)]
            self.bitdec(self, *res)
            self.decomposed = res
            return res + suffix
        else:
            return self.decomposed[:n] + suffix
    @classmethod
    def malloc(cls, size):
        return Program.prog.malloc(size, cls)
    @staticmethod
    def n_elements():
        return 1
    @classmethod
    def load_mem(cls, address, mem_type=None, size=None):
        if size not in (None, 1):
            v = [cls.load_mem(address + i) for i in range(size)]
            return cls.vec(v)
        res = cls()
        if mem_type == 'sd':
            return cls.load_dynamic_mem(address)
        else:
            cls.load_inst[util.is_constant(address)](res, address)
            return res
    def store_in_mem(self, address):
        self.store_inst[isinstance(address, (int, long))](self, address)
    def __init__(self, value=None, n=None, size=None):
        if size != 1 and size is not None:
            raise Exception('invalid size for bit type: %s' % size)
        Tape.Register.__init__(self, self.reg_type, Program.prog.curr_tape)
        self.set_length(n or self.n)
        if value is not None:
            self.load_other(value)
    def set_length(self, n):
        if n > self.max_length:
            print self.max_length
            raise Exception('too long: %d' % n)
        self.n = n
    def load_other(self, other):
        if isinstance(other, (int, long)):
            self.set_length(self.n or util.int_len(other))
            self.load_int(other)
        elif isinstance(other, regint):
            self.conv_regint(self.n, self, other)
        elif isinstance(self, type(other)) or isinstance(other, type(self)):
            self.mov(self, other)
        else:
            try:
                other = self.bit_compose(other.bit_decompose())
                self.mov(self, other)
            except:
                raise CompilerError('cannot convert from %s to %s' % \
                                    (type(other), type(self)))
    def long_one(self):
        return 2**self.n - 1
    def __repr__(self):
        return '%s(%d/%d)' % \
            (super(bits, self).__repr__(), self.n, type(self).n)

class cbits(bits):
    max_length = 64
    reg_type = 'cb'
    is_clear = True
    load_inst = (None, inst.ldmc)
    store_inst = (None, inst.stmc)
    bitdec = inst.bitdecc
    conv_regint = staticmethod(lambda n, x, y: inst.convcint(x, y))
    types = {}
    def load_int(self, value):
        self.load_other(regint(value))
    def store_in_dynamic_mem(self, address):
        inst.stmsdci(self, cbits.conv(address))
    def clear_op(self, other, c_inst, ci_inst, op):
        if isinstance(other, cbits):
            res = cbits(n=max(self.n, other.n))
            c_inst(res, self, other)
            return res
        else:
            if util.is_constant(other):
                if other >= 2**31 or other < -2**31:
                    return op(self, cbits(other))
                res = cbits(n=max(self.n, len(bin(other)) - 2))
                ci_inst(res, self, other)
                return res
            else:
                return op(self, cbits(other))
    __add__ = lambda self, other: \
              self.clear_op(other, inst.addc, inst.addci, operator.add)
    __xor__ = lambda self, other: \
              self.clear_op(other, inst.xorc, inst.xorci, operator.xor)
    __radd__ = __add__
    __rxor__ = __xor__
    def __mul__(self, other):
        if isinstance(other, cbits):
            return NotImplemented
        else:
            try:
                res = cbits(n=min(self.max_length, self.n+util.int_len(other)))
                inst.mulci(res, self, other)
                return res
            except TypeError:
                return NotImplemented
    def __rshift__(self, other):
        res = cbits(n=self.n-other)
        inst.shrci(res, self, other)
        return res
    def __lshift__(self, other):
        res = cbits(n=self.n+other)
        inst.shlci(res, self, other)
        return res
    def print_reg(self, desc=''):
        inst.print_reg(self, desc)
    def print_reg_plain(self):
        inst.print_reg_signed(self.n, self)
    output = print_reg_plain
    def print_if(self, string):
        inst.cond_print_str(self, string)
    def reveal(self):
        return self

class sbits(bits):
    max_length = 128
    reg_type = 'sb'
    is_clear = False
    clear_type = cbits
    default_type = cbits
    load_inst = (inst.ldmsi, inst.ldms)
    store_inst = (inst.stmsi, inst.stms)
    bitdec = inst.bitdecs
    bitcom = inst.bitcoms
    conv_regint = inst.convsint
    mov = inst.movs
    types = {}
    def __init__(self, *args, **kwargs):
        bits.__init__(self, *args, **kwargs)
    @staticmethod
    def new(value=None, n=None):
        if n == 1:
            return sbit(value)
        else:
            return sbits.get_type(n)(value)
    @staticmethod
    def get_random_bit():
        res = sbit()
        inst.bit(res)
        return res
    @classmethod
    def get_input_from(cls, player, n_bits=None):
        if n_bits is None:
            n_bits = cls.n
        res = cls()
        inst.inputb(player, n_bits, res)
        return res
    # compatiblity to sint
    get_raw_input_from = get_input_from
    @classmethod
    def load_dynamic_mem(cls, address):
        res = cls()
        if isinstance(address, (long, int)):
            inst.ldmsd(res, address, cls.n)
        else:
            inst.ldmsdi(res, address, cls.n)
        return res
    def store_in_dynamic_mem(self, address):
        if isinstance(address, (long, int)):
            inst.stmsd(self, address)
        else:
            inst.stmsdi(self, cbits.conv(address))
    def load_int(self, value):
        if (abs(value) > (1 << self.n)):
            raise Exception('public value %d longer than %d bits' \
                            % (value, self.n))
        if self.n <= 32:
            inst.ldbits(self, self.n, value)
        elif self.n <= 64:
            self.load_other(regint(value))
        elif self.n <= 128:
            lower = sbits.get_type(64)(value % 2**64)
            upper = sbits.get_type(self.n - 64)(value >> 64)
            self.mov(self, lower + (upper << 64))
        else:
            raise NotImplementedError('more than 128 bits wanted')
    @read_mem_value
    def __add__(self, other):
        if isinstance(other, int):
            return self.xor_int(other)
        else:
            if not isinstance(other, sbits):
                other = sbits(other)
            n = min(self.n, other.n)
            res = self.new(n=n)
            inst.xors(n, res, self, other)
            max_n = max(self.n, other.n)
            if max_n > n:
                if self.n > n:
                    longer = self
                else:
                    longer = other
                bits = res.bit_decompose() + longer.bit_decompose()[n:]
                res = self.bit_compose(bits)
            return res
    __radd__ = __add__
    __sub__ = __add__
    __xor__ = __add__
    __rxor__ = __add__
    @read_mem_value
    def __rsub__(self, other):
        if isinstance(other, cbits):
            return other + self
        else:
            return self.xor_int(other)
    @read_mem_value
    def __mul__(self, other):
        if isinstance(other, int):
            return self.mul_int(other)
        try:
            if min(self.n, other.n) != 1:
                raise NotImplementedError('high order multiplication')
            n = max(self.n, other.n)
            res = self.new(n=max(self.n, other.n))
            order = (self, other) if self.n != 1 else (other, self)
            inst.andrs(n, res, *order)
            return res
        except AttributeError:
            return NotImplemented
    @read_mem_value
    def __rmul__(self, other):
        if isinstance(other, cbits):
            return other * self
        else:
            return self.mul_int(other)
    @read_mem_value
    def __and__(self, other):
        if util.is_zero(other):
            return 0
        elif util.is_all_ones(other, self.n):
            return self
        assert(self.n == other.n)
        res = self.new(n=self.n)
        inst.ands(self.n, res, self, other)
        return res
    def xor_int(self, other):
        if other == 0:
            return self
        self_bits = self.bit_decompose()
        other_bits = util.bit_decompose(other, max(self.n, util.int_len(other)))
        extra_bits = [self.new(b, n=1) for b in other_bits[self.n:]]
        return self.bit_compose([~x if y else x \
                                 for x,y in zip(self_bits, other_bits)] \
                                + extra_bits)
    def mul_int(self, other):
        if other == 0:
            return 0
        elif other == 1:
            return self
        elif self.n == 1:
            bits = util.bit_decompose(other, util.int_len(other))
            zero = sbit(0)
            mul_bits = [self if b else zero for b in bits]
            return self.bit_compose(mul_bits)
        else:
            print self.n, other
            return NotImplemented
    def __lshift__(self, i):
        return self.bit_compose([sbit(0)] * i + self.bit_decompose()[:self.max_length-i])
    def __invert__(self):
        # res = type(self)(n=self.n)
        # inst.nots(res, self)
        # return res
        one = self.new(value=1, n=1)
        bits = [one + bit for bit in self.bit_decompose()]
        return self.bit_compose(bits)
    def __neg__(self):
        return self
    def reveal(self):
        if self.n > self.clear_type.max_length:
            raise Exception('too long to reveal')
        res = self.clear_type(n=self.n)
        inst.reveal(self.n, res, self)
        return res
    def equal(self, other, n=None):
        bits = (~(self + other)).bit_decompose()
        return reduce(operator.mul, bits)
    def TruncPr(self, k, m, kappa=None):
        if k > self.n:
            raise Exception('TruncPr overflow: %d > %d' % (k, self.n))
        bits = self.bit_decompose()
        res = self.get_type(k - m).bit_compose(bits[m:k])
        return res
    @classmethod
    def two_power(cls, n):
        if n > cls.n:
            raise Exception('two_power overflow: %d > %d' % (n, cls.n))
        res = cls()
        if n == cls.n:
            res.load_int(-1 << (n - 1))
        else:
            res.load_int(1 << n)
        return res
    def popcnt(self):
        return sbitvec(self).popcnt().elements()[0]
    @classmethod
    def trans(cls, rows):
        rows = list(rows)
        if len(rows) == 1:
            return rows[0].bit_decompose()
        n_columns = rows[0].n
        for row in rows:
            assert(row.n == n_columns)
        if n_columns == 1:
            return [cls.bit_compose(rows)]
        else:
            res = [cls.new(n=len(rows)) for i in range(n_columns)]
            inst.trans(len(res), *(res + rows))
            return res
    def if_else(self, x, y):
        # vectorized if/else
        return result_conv(x, y)(self & (x ^ y) ^ y)

class sbitvec(object):
    @classmethod
    def get_type(cls, n):
        return cls
    @classmethod
    def from_vec(cls, vector):
        res = cls()
        res.v = list(vector)
        return res
    @classmethod
    def combine(cls, vectors):
        res = cls()
        res.v = sum((vec.v for vec in vectors), [])
        return res
    @classmethod
    def from_matrix(cls, matrix):
        # any number of rows, limited number of columns
        return cls.combine(cls(row) for row in matrix)
    def __init__(self, elements=None):
        if elements is not None:
            self.v = sbits.trans(elements)
    def popcnt(self):
        res = sbitint.wallace_tree([[b] for b in self.v])
        while res[-1] is 0:
            del res[-1]
        return self.from_vec(res)
    def elements(self, start=None, stop=None):
        if stop is None:
            start, stop = stop, start
        return sbits.trans(self.v[start:stop])
    def __xor__(self, other):
        return self.from_vec(x ^ y for x, y in zip(self.v, other.v))
    def __and__(self, other):
        return self.from_vec(x & y for x, y in zip(self.v, other.v))
    def if_else(self, x, y):
        assert(len(self.v) == 1)
        try:
            return self.from_vec(util.if_else(self.v[0], a, b) \
                                 for a, b in zip(x, y))
        except:
            return util.if_else(self.v[0], x, y)
    def __iter__(self):
        return iter(self.v)
    def __len__(self):
        return len(self.v)
    @classmethod
    def conv(cls, other):
        return cls.from_vec(other.v)
    @property
    def size(self):
        return self.v[0].n
    def store_in_mem(self, address):
        for i, x in enumerate(self.elements()):
            x.store_in_mem(address + i)
    def bit_decompose(self):
        return self.v
    bit_compose = from_vec

class bit(object):
    n = 1
    
def result_conv(x, y):
    try:
        if util.is_constant(x):
            if util.is_constant(y):
                return lambda x: x
            else:
                return type(y).conv
        if util.is_constant(y):
            return type(x).conv
        if type(x) is type(y):
            return type(x).conv
    except AttributeError:
        pass
    return lambda x: x

class sbit(bit, sbits):
    def if_else(self, x, y):
        return result_conv(x, y)(self * (x ^ y) ^ y)

class cbit(bit, cbits):
    pass

sbits.bit_type = sbit
cbits.bit_type = cbit

class bitsBlock(oram.Block):
    value_type = sbits
    def __init__(self, value, start, lengths, entries_per_block):
        oram.Block.__init__(self, value, lengths)
        length = sum(self.lengths)
        used_bits = entries_per_block * length
        self.value_bits = self.value.bit_decompose(used_bits)
        start_length = util.log2(entries_per_block)
        self.start_bits = util.bit_decompose(start, start_length)
        self.start_demux = oram.demux_list(self.start_bits)
        self.entries = [sbits.bit_compose(self.value_bits[i*length:][:length]) \
                        for i in range(entries_per_block)]
        self.mul_entries = map(operator.mul, self.start_demux, self.entries)
        self.bits = sum(self.mul_entries).bit_decompose()
        self.mul_value = sbits.compose(self.mul_entries, sum(self.lengths))
        self.anti_value = self.mul_value + self.value
    def set_slice(self, value):
        value = sbits.compose(util.tuplify(value), sum(self.lengths))
        for i,b in enumerate(self.start_bits):
            value = b.if_else(value << (2**i * sum(self.lengths)), value)
        self.value = value + self.anti_value
        return self

oram.block_types[sbits] = bitsBlock

class dyn_sbits(sbits):
    pass

class DynamicArray(Array):
    def __init__(self, *args):
        Array.__init__(self, *args)
    def _malloc(self):
        return Program.prog.malloc(self.length, 'sd', self.value_type)
    def _load(self, address):
        return self.value_type.load_dynamic_mem(cbits.conv(address))
    def _store(self, value, address):
        if isinstance(value, MemValue):
            value = value.read()
        if isinstance(value, sbits):
            self.value_type.conv(value).store_in_dynamic_mem(address)
        else:
            cbits.conv(value).store_in_dynamic_mem(address)

sbits.dynamic_array = DynamicArray
cbits.dynamic_array = Array

class sbitint(_bitint, _number, sbits):
    n_bits = None
    bin_type = None
    types = {}
    @classmethod
    def get_type(cls, n, other=None):
        if isinstance(other, sbitvec):
            return sbitvec
        if n in cls.types:
            return cls.types[n]
        sbits_type = sbits.get_type(n)
        class _(sbitint, sbits_type):
            # n_bits is used by _bitint
            n_bits = n
            bin_type = sbits_type
        _.__name__ = 'sbitint' + str(n)
        cls.types[n] = _
        return _
    @classmethod
    def combo_type(cls, other):
        if isinstance(other, sbitintvec):
            return sbitintvec
        else:
            return cls
    @classmethod
    def new(cls, value=None, n=None):
        return cls.get_type(n)(value)
    def set_length(*args):
        pass
    @classmethod
    def bit_compose(cls, bits):
        # truncate and extend bits
        bits = bits[:cls.n]
        bits += [0] * (cls.n - len(bits))
        return super(sbitint, cls).bit_compose(bits)
    def force_bit_decompose(self, n_bits=None):
        return sbits.bit_decompose(self, n_bits)
    def TruncMul(self, other, k, m, kappa=None, nearest=False):
        if nearest:
            raise CompilerError('round to nearest not implemented')
        self_bits = self.bit_decompose()
        other_bits = other.bit_decompose()
        if len(self_bits) + len(other_bits) != k:
            raise Exception('invalid parameters for TruncMul: '
                            'self:%d, other:%d, k:%d' %
                            (len(self_bits), len(other_bits), k))
        t = self.get_type(k)
        a = t.bit_compose(self_bits + [self_bits[-1]] * (k - len(self_bits)))
        t = other.get_type(k)
        b = t.bit_compose(other_bits + [other_bits[-1]] * (k - len(other_bits)))
        product = a * b
        res_bits = product.bit_decompose()[m:k]
        t = self.combo_type(other)
        return t.bit_compose(res_bits)
    def Norm(self, k, f, kappa=None, simplex_flag=False):
        absolute_val = abs(self)
        #next 2 lines actually compute the SufOR for little indian encoding
        bits = absolute_val.bit_decompose(k)[::-1]
        suffixes = floatingpoint.PreOR(bits)[::-1]
        z = [0] * k
        for i in range(k - 1):
            z[i] = suffixes[i] - suffixes[i+1]
        z[k - 1] = suffixes[k-1]
        z.reverse()
        t2k = self.get_type(2 * k)
        acc = t2k.bit_compose(z)
        sign = self.bit_decompose()[-1]
        signed_acc = sign.if_else(-acc, acc)
        absolute_val_2k = t2k.bit_compose(absolute_val.bit_decompose())
        part_reciprocal = absolute_val_2k * acc
        return part_reciprocal, signed_acc
    def extend(self, n):
        bits = self.bit_decompose()
        bits += [bits[-1]] * (n - len(bits))
        return self.get_type(n).bit_compose(bits)
    def __mul__(self, other):
        if isinstance(other, sbitintvec):
            return other * self
        else:
            return super(sbitint, self).__mul__(other)

class sbitintvec(sbitvec):
    def __add__(self, other):
        if other is 0:
            return self
        assert(len(self.v) == len(other.v))
        v = sbitint.bit_adder(self.v, other.v)
        return self.from_vec(v)
    __radd__ = __add__
    def less_than(self, other, *args, **kwargs):
        assert(len(self.v) == len(other.v))
        return self.from_vec(sbitint.bit_less_than(self.v, other.v))
    def __mul__(self, other):
        assert isinstance(other, sbitint)
        matrix = [[x * b for x in self.v] for b in other.bit_decompose()]
        v = sbitint.wallace_tree_from_matrix(matrix)
        return self.from_vec(v)
    __rmul__ = __mul__
    reduce_after_mul = lambda x: x

sbitint.vec = sbitintvec

class cbitfix(object):
    def __init__(self, value):
        self.v = value
    def output(self):
        bits = self.v.bit_decompose(self.k)
        sign = bits[-1]
        v = self.v + (sign << (self.k)) * -1
        inst.print_float_plain(v, cbits(-self.f, n=32), cbits(0), cbits(0))

class sbitfix(_fix):
    float_type = type(None)
    clear_type = cbitfix
    @classmethod
    def set_precision(cls, f, k=None):
        super(cls, sbitfix).set_precision(f, k)
        cls.int_type = sbitint.get_type(cls.k)
    @classmethod
    def load_mem(cls, address, size=None):
        if size not in (None, 1):
            v = [cls.int_type.load_mem(address + i) for i in range(size)]
            return sbitfixvec._new(sbitintvec(v))
        else:
            return super(sbitfix, cls).load_mem(address)
    def __xor__(self, other):
        return type(self)(self.v ^ other.v)
    def __mul__(self, other):
        if isinstance(other, sbit):
            return type(self)(self.int_type(other * self.v))
        else:
            return super(sbitfix, self).__mul__(other)
    __rxor__ = __xor__
    __rmul__ = __mul__

sbitfix.set_precision(20, 41)

class sbitfixvec(_fix):
    int_type = sbitintvec
    float_type = type(None)
    @staticmethod
    @property
    def f():
        return sbitfix.f
    @staticmethod
    @property
    def k():
        return sbitfix.k

sbitfix.vec = sbitfixvec
