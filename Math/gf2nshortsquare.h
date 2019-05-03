/*
 * gf2nSquare.h
 *
 */

#ifndef MATH_GF2NSHORTSQUARE_H_
#define MATH_GF2NSHORTSQUARE_H_

#include "Math/gf2n.h"
#include "OT/BitVector.h"

class gf2n_short_square
{
public:
    static const int N_ROWS = 64;
    static const int N_ROWS_ALLOCATED = 64;
    static const int N_COLUMNS = 64;
    static const int N_ROW_BYTES = sizeof(gf2n_short);

    static size_t size() { return N_ROWS * gf2n_short::size(); }

    gf2n_short rows[N_ROWS];

    template<class T>
    gf2n_short_square& sub(const gf2n_short_square& other);
    template<class T>
    gf2n_short_square& rsub(const gf2n_short_square& other) { return sub<T>(other); }
    template<class T>
    gf2n_short_square& sub(const void* other);

    template <class T>
    void randomize(int row, PRNG& G) { rows[row].randomize(G); }
    template <class T>
    void conditional_add(BitVector& conditions, gf2n_short_square& other,
            int offset);
    void to(gf2n_short& result);

    void pack(octetStream& os) const;
    void unpack(octetStream& os);
};

#endif /* MATH_GF2NSHORTSQUARE_H_ */
