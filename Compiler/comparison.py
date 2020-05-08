"""
Functions for secure comparison of GF(p) types.
Most protocols come from [1], with a few subroutines described in [2].

Function naming of comparison routines is as in [1,2], with k always
representing the integer bit length, and kappa the statistical security
parameter.

Most of these routines were implemented before the cint/sint classes, so use
the old-fasioned Register class and assembly instructions instead of operator
overloading.

The PreMulC function has a few variants, depending on whether
preprocessing is only triples/bits, or inverse tuples or "special"
comparison-specific preprocessing is also available.

[1] https://www1.cs.fau.de/filepool/publications/octavian_securescm/smcint-scn10.pdf
[2] https://www1.cs.fau.de/filepool/publications/octavian_securescm/SecureSCM-D.9.2.pdf
"""

# Use constant rounds protocols instead of log rounds
const_rounds = False
# Set use_inv to use preprocessed inverse tuples for more efficient
# online phase comparisons.
use_inv = True
# If do_precomp is not set, use_inv uses standard inverse tuples, otherwise if
# both are set, use a list of "special" tuples of the form
# (r[i], r[i]^-1, r[i] * r[i-1]^-1)
do_precomp = True

from . import instructions_base
from . import util

def set_variant(options):
    """ Set flags based on the command-line option provided """
    global const_rounds, do_precomp, use_inv
    variant = options.comparison
    if variant == 'log':
        const_rounds = False
    elif variant == 'plain':
        const_rounds = True
        use_inv = False
    elif variant == 'inv':
        const_rounds = True
        use_inv = True
        do_precomp = True
    elif variant == 'sinv':
        const_rounds = True
        use_inv = True
        do_precomp = False
    elif variant is not None:
        raise CompilerError('Unknown comparison variant: %s' % variant)

def ld2i(c, n):
    """ Load immediate 2^n into clear GF(p) register c """
    t1 = program.curr_block.new_reg('c')
    ldi(t1, 2 ** (n % 30))
    for i in range(n // 30):
        t2 = program.curr_block.new_reg('c')
        mulci(t2, t1, 2 ** 30)
        t1 = t2
    movc(c, t1)

inverse_of_two = {}

def divide_by_two(res, x, m=1):
    """ Faster clear division by two using a cached value of 2^-1 mod p """
    tmp = program.curr_block.new_reg('c')
    inv2m(tmp, m)
    mulc(res, x, tmp)

@instructions_base.cisc
def LTZ(s, a, k, kappa):
    """
    s = (a ?< 0)

    k: bit length of a
    """
    from .types import sint, _bitint
    from .GC.types import sbitvec
    if program.use_split():
        movs(s, sint.conv(sbitvec(a, k).v[-1]))
        return
    elif program.options.ring:
        from . import floatingpoint
        assert(int(program.options.ring) >= k)
        m = k - 1
        shift = int(program.options.ring) - k
        r_prime, r_bin = MaskingBitsInRing(k)
        tmp = a - r_prime
        c_prime = (tmp << shift).reveal() >> shift
        a = r_bin[0].bit_decompose_clear(c_prime, m)
        b = r_bin[:m]
        u = CarryOutRaw(a[::-1], b[::-1])
        movs(s, sint.conv(r_bin[m].bit_xor(c_prime >> m).bit_xor(u)))
        return
    t = sint()
    Trunc(t, a, k, k - 1, kappa, True)
    subsfi(s, t, 0)

def LessThanZero(a, k, kappa):
    from . import types
    res = types.sint()
    LTZ(res, a, k, kappa)
    return res

@instructions_base.cisc
def Trunc(d, a, k, m, kappa, signed):
    """
    d = a >> m

    k: bit length of a
    m: compile-time integer
    signed: True/False, describes a
    """
    a_prime = program.curr_block.new_reg('s')
    t = program.curr_block.new_reg('s')
    c = [program.curr_block.new_reg('c') for i in range(3)]
    c2m = program.curr_block.new_reg('c')
    if m == 0:
        movs(d, a)
        return
    elif program.options.ring:
        return TruncRing(d, a, k, m, signed)
    elif m == 1:
        Mod2(a_prime, a, k, kappa, signed)
    else:
        Mod2m(a_prime, a, k, m, kappa, signed)
    subs(t, a, a_prime)
    ldi(c[1], 1)
    divide_by_two(c[2], c[1], m)
    mulm(d, t, c[2])

def TruncRing(d, a, k, m, signed):
    a_prime = Mod2mRing(None, a, k, m, signed)
    a -= a_prime
    res = TruncLeakyInRing(a, k, m, signed)
    if d is not None:
        movs(d, res)
    return res

def TruncZeros(a, k, m, signed):
    if program.options.ring:
        return TruncLeakyInRing(a, k, m, signed)
    else:
        from . import types
        tmp = types.cint()
        inv2m(tmp, m)
        return a * tmp

def TruncLeakyInRing(a, k, m, signed):
    """
    Returns a >> m.
    Requires a < 2^k and leaks a % 2^m (needs to be constant or random).
    """
    assert k > m
    assert int(program.options.ring) >= k
    from .types import sint, intbitint, cint, cgf2n
    n_bits = k - m
    n_shift = int(program.options.ring) - n_bits
    if n_bits > 1:
        r, r_bits = MaskingBitsInRing(n_bits, True)
    else:
        r_bits = [sint.get_random_bit() for i in range(n_bits)]
        r = sint.bit_compose(r_bits)
    if signed:
        a += (1 << (k - 1))
    shifted = ((a << (n_shift - m)) + (r << n_shift)).reveal()
    masked = shifted >> n_shift
    u = sint()
    BitLTL(u, masked, r_bits[:n_bits], 0)
    res = (u << n_bits) + masked - r
    if signed:
        res -= (1 << (n_bits - 1))
    return res

def TruncRoundNearest(a, k, m, kappa, signed=False):
    """
    Returns a / 2^m, rounded to the nearest integer.

    k: bit length of a
    m: compile-time integer
    """
    if m == 0:
        return a
    if k == int(program.options.ring):
        # cannot work with bit length k+1
        tmp = TruncRing(None, a, k, m - 1, signed)
        return TruncRing(None, tmp + 1, k - m + 1, 1, signed)
    from .types import sint
    res = sint()
    Trunc(res, a + (1 << (m - 1)), k + 1, m, kappa, signed)
    return res

@instructions_base.cisc
def Mod2m(a_prime, a, k, m, kappa, signed):
    """
    a_prime = a % 2^m

    k: bit length of a
    m: compile-time integer
    signed: True/False, describes a
    """
    if not util.is_constant(m):
        raise CompilerError('m must be a public constant')
    if m >= k:
        movs(a_prime, a)
        return
    if program.options.ring:
        return Mod2mRing(a_prime, a, k, m, signed)
    else:
        if m == 1:
            return Mod2(a_prime, a, k, kappa, signed)
        else:
            return Mod2mField(a_prime, a, k, m, kappa, signed)

def Mod2mRing(a_prime, a, k, m, signed):
    assert(int(program.options.ring) >= k)
    from Compiler.types import sint, intbitint, cint
    shift = int(program.options.ring) - m
    r_prime, r_bin = MaskingBitsInRing(m, True)
    tmp = a + r_prime
    c_prime = (tmp << shift).reveal() >> shift
    u = sint()
    BitLTL(u, c_prime, r_bin[:m], 0)
    res = (u << m) + c_prime - r_prime
    if a_prime is not None:
        movs(a_prime, res)
    return res

def Mod2mField(a_prime, a, k, m, kappa, signed):
    from .types import sint
    r_dprime = program.curr_block.new_reg('s')
    r_prime = program.curr_block.new_reg('s')
    r = [sint() for i in range(m)]
    c = program.curr_block.new_reg('c')
    c_prime = program.curr_block.new_reg('c')
    v = program.curr_block.new_reg('s')
    u = program.curr_block.new_reg('s')
    t = [program.curr_block.new_reg('s') for i in range(6)]
    c2m = program.curr_block.new_reg('c')
    c2k1 = program.curr_block.new_reg('c')
    PRandM(r_dprime, r_prime, r, k, m, kappa)
    ld2i(c2m, m)
    mulm(t[0], r_dprime, c2m)
    if signed:
        ld2i(c2k1, k - 1)
        addm(t[1], a, c2k1)
    else:
        t[1] = a
    adds(t[2], t[0], t[1])
    adds(t[3], t[2], r_prime)
    asm_open(c, t[3])
    modc(c_prime, c, c2m)
    if const_rounds:
        BitLTC1(u, c_prime, r, kappa)
    else:
        BitLTL(u, c_prime, r, kappa)
    mulm(t[4], u, c2m)
    submr(t[5], c_prime, r_prime)
    adds(a_prime, t[5], t[4])
    return r_dprime, r_prime, c, c_prime, u, t, c2k1

def MaskingBitsInRing(m, strict=False):
    program.curr_tape.require_bit_length(1)
    from Compiler.types import sint
    if program.use_edabit():
        return sint.get_edabit(m, strict)
    elif program.use_dabit:
        r, r_bin = zip(*(sint.get_dabit() for i in range(m)))
    else:
        r = [sint.get_random_bit() for i in range(m)]
        r_bin = r
    return sint.bit_compose(r), r_bin

def PRandM(r_dprime, r_prime, b, k, m, kappa, use_dabit=True):
    """
    r_dprime = random secret integer in range [0, 2^(k + kappa - m) - 1]
    r_prime = random secret integer in range [0, 2^m - 1]
    b = array containing bits of r_prime
    """
    program.curr_tape.require_bit_length(k + kappa)
    from .types import sint
    if program.use_edabit() and m > 1:
        movs(r_dprime, sint.get_edabit(k + kappa - m, True)[0])
        tmp, b[:] = sint.get_edabit(m, True)
        movs(r_prime, tmp)
        return
    t = [[program.curr_block.new_reg('s') for j in range(2)] for i in range(m)]
    t[0][1] = b[-1]
    PRandInt(r_dprime, k + kappa - m)
    # r_dprime is always multiplied by 2^m
    if use_dabit and program.use_dabit and m > 1:
        r, b[:] = zip(*(sint.get_dabit() for i in range(m)))
        r = sint.bit_compose(r)
        movs(r_prime, r)
        return
    bit(b[-1])
    for i in range(1,m):
        adds(t[i][0], t[i-1][1], t[i-1][1])
        bit(b[-i-1])
        adds(t[i][1], t[i][0], b[-i-1])
    movs(r_prime, t[m-1][1])

def PRandInt(r, k):
    """
    r = random secret integer in range [0, 2^k - 1]
    """
    t = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(3)]
    t[2][k-1] = r
    bit(t[2][0])
    for i in range(1,k):
        adds(t[0][i], t[2][i-1], t[2][i-1])
        bit(t[1][i])
        adds(t[2][i], t[0][i], t[1][i])

def BitLTC1(u, a, b, kappa):
    """
    u = a <? b

    a: array of clear bits
    b: array of secret bits (same length as a)
    """
    k = len(b)
    p = [program.curr_block.new_reg('s') for i in range(k)]
    from . import floatingpoint
    a_bits = floatingpoint.bits(a, k)
    if instructions_base.get_global_vector_size() == 1:
        a_ = a_bits
        a_bits = program.curr_block.new_reg('c', size=k)
        b_vec = program.curr_block.new_reg('s', size=k)
        for i in range(k):
            movc(a_bits[i], a_[i])
            movs(b_vec[i], b[i])
        d = program.curr_block.new_reg('s', size=k)
        s = program.curr_block.new_reg('s', size=k)
        t = [program.curr_block.new_reg('s', size=k) for j in range(5)]
        c = [program.curr_block.new_reg('c', size=k) for j in range(4)]
    else:
        d = [program.curr_block.new_reg('s') for i in range(k)]
        s = [program.curr_block.new_reg('s') for i in range(k)]
        t = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(5)]
        c = [[program.curr_block.new_reg('c') for i in range(k)] for j in range(4)]
    if instructions_base.get_global_vector_size() == 1:
        vmulci(k, c[2], a_bits, 2)
        vmulm(k, t[0], b_vec, c[2])
        vaddm(k, t[1], b_vec, a_bits)
        vsubs(k, d, t[1], t[0])
        vaddsi(k, t[2], d, 1)
        t[2].create_vector_elements()
        pre_input = t[2].vector[:]
    else:
        for i in range(k):
            mulci(c[2][i], a_bits[i], 2)
            mulm(t[0][i], b[i], c[2][i])
            addm(t[1][i], b[i], a_bits[i])
            subs(d[i], t[1][i], t[0][i])
            addsi(t[2][i], d[i], 1)
            pre_input = t[2][:]
    pre_input.reverse()
    if use_inv:
        if instructions_base.get_global_vector_size() == 1:
            PreMulC_with_inverses_and_vectors(p, pre_input)
        else:
            if do_precomp:
                PreMulC_with_inverses(p, pre_input)
            else:
                raise NotImplementedError('Vectors not compatible with -c sinv')
    else:
        PreMulC_without_inverses(p, pre_input)
    p.reverse()
    for i in range(k-1):
        subs(s[i], p[i], p[i+1])
    subsi(s[k-1], p[k-1], 1)
    subcfi(c[3][0], a_bits[0], 1)
    mulm(t[4][0], s[0], c[3][0])
    from .types import sint
    t[3] = [sint() for i in range(k)]
    for i in range(1,k):
        subcfi(c[3][i], a_bits[i], 1)
        mulm(t[3][i], s[i], c[3][i])
        adds(t[4][i], t[4][i-1], t[3][i])
    Mod2(u, t[4][k-1], k, kappa, False)
    return p, a_bits, d, s, t, c, b, pre_input

def carry(b, a, compute_p):
    """ Carry propogation:
        return (p,g) = (p_2, g_2)o(p_1, g_1) -> (p_1 & p_2, g_2 | (p_2 & g_1))
    """
    if a is None:
        return b
    if b is None:
        return a
    t = [program.curr_block.new_reg('s') for i in range(3)]
    if compute_p:
        t[0] = a[0].bit_and(b[0])
    t[2] = a[0].bit_and(b[1]) + a[1]
    return t[0], t[2]

# from WP9 report
# length of a is even
def CarryOutAux(a, kappa):
    k = len(a)
    if k > 1 and k % 2 == 1:
        a.append(None)
        k += 1
    u = [None]*(k//2)
    a = a[::-1]
    if k > 1:
        for i in range(k//2):
            u[i] = carry(a[2*i+1], a[2*i], i != k//2-1)
        return CarryOutAux(u[:k//2][::-1], kappa)
    else:
        return a[0][1]

# carry out with carry-in bit c
def CarryOut(res, a, b, c=0, kappa=None):
    """
    res = last carry bit in addition of a and b

    a: array of clear bits
    b: array of secret bits (same length as a)
    c: initial carry-in bit
    """
    from .types import sint
    movs(res, sint.conv(CarryOutRaw(a, b, c)))

def CarryOutRaw(a, b, c=0):
    assert len(a) == len(b)
    k = len(a)
    from . import types
    d = [program.curr_block.new_reg('s') for i in range(k)]
    s = [program.curr_block.new_reg('s') for i in range(3)]
    for i in range(k):
        d[i] = list(b[i].half_adder(a[i]))
    s[0] = d[-1][0].bit_and(c)
    s[1] = d[-1][1] + s[0]
    d[-1][1] = s[1]
    return CarryOutAux(d[::-1], None)

def CarryOutRawLE(a, b, c=0):
    """ Little-endian version """
    return CarryOutRaw(a[::-1], b[::-1], c)

def CarryOutLE(a, b, c=0):
    """ Little-endian version """
    from . import types
    res = types.sint()
    CarryOut(res, a[::-1], b[::-1], c)
    return res

def BitLTL(res, a, b, kappa):
    """
    res = a <? b (logarithmic rounds version)

    a: clear integer register
    b: array of secret bits (same length as a)
    """
    k = len(b)
    a_bits = b[0].bit_decompose_clear(a, k)
    s = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(2)]
    t = [program.curr_block.new_reg('s') for i in range(1)]
    for i in range(len(b)):
        s[0][i] = b[0].long_one() - b[i]
    CarryOut(t[0], a_bits[::-1], s[0][::-1], b[0].long_one(), kappa)
    subsfi(res, t[0], 1)
    return a_bits, s[0]

def PreMulC_with_inverses_and_vectors(p, a):
    """
    p[i] = prod_{j=0}^{i-1} a[i]

    Variant for vector registers using preprocessed inverses.
    """
    k = len(p)
    a_vec = program.curr_block.new_reg('s', size=k)
    r = program.curr_block.new_reg('s', size=k)
    w = program.curr_block.new_reg('s', size=k)
    w_tmp = program.curr_block.new_reg('s', size=k)
    z = program.curr_block.new_reg('s', size=k)
    m = program.curr_block.new_reg('c', size=k)
    t = [program.curr_block.new_reg('s', size=k) for i in range(1)]
    c = [program.curr_block.new_reg('c') for i in range(k)]
    # warning: computer scientists count from 0
    if do_precomp:
        vinverse(k, r, z)
    else:
        vprep(k, 'PreMulC', r, z, w_tmp)
    for i in range(1,k):
        if do_precomp:
            muls(w[i], r[i], z[i-1])
        else:
            movs(w[i], w_tmp[i])
        movs(a_vec[i], a[i])
    movs(w[0], r[0])
    movs(a_vec[0], a[0])
    vmuls(k, t[0], w, a_vec)
    vasm_open(k, m, t[0])
    PreMulC_end(p, a, c, m, z)

def PreMulC_with_inverses(p, a):
    """
    Variant using preprocessed inverses or special inverses.
    The latter are triples of the form (a_i, a_i^{-1}, a_i * a_{i-1}^{-1}).
    See also make_PreMulC() in Fake-Offline.cpp.
    """
    k = len(a)
    r = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(3)]
    w = [[program.curr_block.new_reg('s') for i in range(k)] for j in range(2)]
    z = [program.curr_block.new_reg('s') for i in range(k)]
    m = [program.curr_block.new_reg('c') for i in range(k)]
    t = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(1)]
    c = [program.curr_block.new_reg('c') for i in range(k)]
    # warning: computer scientists count from 0
    for i in range(k):
        if do_precomp:
            inverse(r[0][i], z[i])
        else:
            prep('PreMulC', r[0][i], z[i], w[1][i])
    if do_precomp:
        for i in range(1,k):
            muls(w[1][i], r[0][i], z[i-1])
    w[1][0] = r[0][0]
    for i in range(k):
        muls(t[0][i], w[1][i], a[i])
        asm_open(m[i], t[0][i])
    PreMulC_end(p, a, c, m, z)

def PreMulC_without_inverses(p, a):
    """
    Plain variant with no extra preprocessing.
    """
    k = len(a)
    r = [program.curr_block.new_reg('s') for i in range(k)]
    s = [program.curr_block.new_reg('s') for i in range(k)]
    u = [program.curr_block.new_reg('c') for i in range(k)]
    v = [program.curr_block.new_reg('s') for i in range(k)]
    w = [program.curr_block.new_reg('s') for i in range(k)]
    z = [program.curr_block.new_reg('s') for i in range(k)]
    m = [program.curr_block.new_reg('c') for i in range(k)]
    t = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(2)]
    #tt = [[program.curr_block.new_reg('s') for i in range(k)] for i in range(4)]
    u_inv = [program.curr_block.new_reg('c') for i in range(k)]
    c = [program.curr_block.new_reg('c') for i in range(k)]
    # warning: computer scientists count from 0
    for i in range(k):
        triple(s[i], r[i], t[0][i])
        #adds(tt[0][i], t[0][i], a[i])
        #subs(tt[1][i], tt[0][i], a[i])
        #startopen(tt[1][i])
        asm_open(u[i], t[0][i])
    for i in range(k-1):
        muls(v[i], r[i+1], s[i])
    w[0] = r[0]
    one = program.curr_block.new_reg('c')
    ldi(one, 1)
    for i in range(k):
        divc(u_inv[i], one, u[i])
        # avoid division by zero, just for benchmarking
        #divc(u_inv[i], u[i], one)
    for i in range(1,k):
        mulm(w[i], v[i-1], u_inv[i-1])
    for i in range(1,k):
        mulm(z[i], s[i], u_inv[i])
    for i in range(k):
        muls(t[1][i], w[i], a[i])
        asm_open(m[i], t[1][i])
    PreMulC_end(p, a, c, m, z)

def PreMulC_end(p, a, c, m, z):
    """
    Helper function for all PreMulC variants. Local operation.
    """
    k = len(a)
    c[0] = m[0]
    for j in range(1,k):
        mulc(c[j], c[j-1], m[j])
        if isinstance(p, list):
            mulm(p[j], z[j], c[j])
    if isinstance(p, list):
        p[0] = a[0]
    else:
        mulm(p, z[-1], c[-1])

def PreMulC(a):
    p = [type(a[0])() for i in range(len(a))]
    instructions_base.set_global_instruction_type(a[0].instruction_type)
    if use_inv:
        PreMulC_with_inverses(p, a)
    else:
        PreMulC_without_inverses(p, a)
    instructions_base.reset_global_instruction_type()
    return p

def KMulC(a):
    """
    Return just the product of all items in a
    """
    from .types import sint, cint
    p = sint()
    if use_inv:
        PreMulC_with_inverses(p, a)
    else:
        PreMulC_without_inverses(p, a)
    return p

def Mod2(a_0, a, k, kappa, signed):
    """
    a_0 = a % 2

    k: bit length of a
    """
    if k <= 1:
        movs(a_0, a)
        return
    r_dprime = program.curr_block.new_reg('s')
    r_prime = program.curr_block.new_reg('s')
    r_0 = program.curr_block.new_reg('s')
    c = program.curr_block.new_reg('c')
    c_0 = program.curr_block.new_reg('c')
    tc = program.curr_block.new_reg('c')
    t = [program.curr_block.new_reg('s') for i in range(6)]
    c2k1 = program.curr_block.new_reg('c')
    PRandM(r_dprime, r_prime, [r_0], k, 1, kappa)
    mulsi(t[0], r_dprime, 2)
    if signed:
        ld2i(c2k1, k - 1)
        addm(t[1], a, c2k1)
    else:
        t[1] = a
    adds(t[2], t[0], t[1])
    adds(t[3], t[2], r_prime)
    asm_open(c, t[3])
    from . import floatingpoint
    c_0 = floatingpoint.bits(c, 1)[0]
    mulci(tc, c_0, 2)
    mulm(t[4], r_0, tc)
    addm(t[5], r_0, c_0)
    subs(a_0, t[5], t[4])


# hack for circular dependency
from .instructions import *
