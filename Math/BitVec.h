/*
 * BitVec.h
 *
 */

#ifndef MATH_BITVEC_H_
#define MATH_BITVEC_H_

#include "Integer.h"
#include "field_types.h"
#include "Square.h"

class BitDiagonal;

class BitVec : public IntBase
{
public:
    typedef BitVec Scalar;

    typedef BitVec next;
    typedef BitDiagonal Square;

    static const int n_bits = sizeof(a) * 8;

    static char type_char() { return 'B'; }
    static DataFieldType field_type() { return DATA_GF2; }

    static bool allows(Dtype dtype) { return dtype == DATA_TRIPLE or dtype == DATA_BIT; }

    BitVec() {}
    BitVec(long a) : IntBase(a) {}
    BitVec(const IntBase& a) : IntBase(a) {}

    BitVec operator+(const BitVec& other) const { return a ^ other.a; }
    BitVec operator-(const BitVec& other) const { return a ^ other.a; }
    BitVec operator*(const BitVec& other) const { return a & other.a; }

    BitVec operator/(const BitVec& other) const { (void) other; throw not_implemented(); }

    BitVec& operator+=(const BitVec& other) { *this ^= other; return *this; }
    BitVec& operator-=(const BitVec& other) { *this ^= other; return *this; }

    BitVec extend_bit() const { return -(a & 1); }
    BitVec mask(int n) const { return n < n_bits ? *this & ((1L << n) - 1) : *this; }

    template<int t>
    void add(octetStream& os) { *this += os.get<BitVec>(); }

    void mul(const BitVec& a, const BitVec& b) { *this = a * b; }

    void randomize(PRNG& G, int n = n_bits) { IntBase::randomize(G); *this = mask(n); }

    void pack(octetStream& os, int n = n_bits) const { os.store_int(a, DIV_CEIL(n, 8)); }
    void unpack(octetStream& os, int n = n_bits) { a = os.get_int(DIV_CEIL(n, 8)); }

    static BitVec unpack_new(octetStream& os, int n = n_bits)
    {
        BitVec res;
        res.unpack(os, n);
        return res;
    }
};

#endif /* MATH_BITVEC_H_ */
