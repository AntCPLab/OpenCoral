#ifndef TOOLS_MPDZ_NTL_TYPES_H_
#define TOOLS_MPDZ_NTL_TYPES_H_

#include "Math/gf2n.h"
#include "Math/mfe.h"
#include "Math/BitVec.h"
#include "Tools/BitVector.h"

inline void ntl_gf2x_to_mpdz_gf2n(gf2n_short& y, const NTL::GF2X& x) {
    if (deg(x) >= gf2n_short::degree())
        throw runtime_error("Cannot convert NTL::GF2X (deg: " + to_string(deg(x)) 
            + ") to a smaller gf2n element (deg: " + to_string(gf2n_short::degree()) + ")");
    // empty poly returns deg(*)=-1, but it actually means 0
    if (deg(x) == -1)
        y = gf2n_short(0);
    else
        y = gf2n_short(x.xrep[0]);
}

inline void conv(gf2n_short& y, const NTL::GF2X& x) {
    ntl_gf2x_to_mpdz_gf2n(y, x);
}

inline gf2n_short ntl_gf2x_to_mpdz_gf2n(const NTL::GF2X& x) {
    gf2n_short y;
    ntl_gf2x_to_mpdz_gf2n(y, x);
    return y;
}

inline void mpdz_gf2n_to_ntl_gf2x(NTL::GF2X& y, const gf2n_short& x) {
    if (x.get_word() == 0) {
        clear(y);
    }
    else {
        y.xrep.SetLength(1);
        y.xrep[0] = x.get_word();
    }
}

inline void conv(NTL::GF2X& y, const gf2n_short& x) {
    mpdz_gf2n_to_ntl_gf2x(y, x);
}

inline NTL::GF2X mpdz_gf2n_to_ntl_gf2x(const gf2n_short& x) {
    NTL::GF2X y;
    mpdz_gf2n_to_ntl_gf2x(y, x);
    return y;
}

inline void conv(BitVec& y, const NTL::vec_GF2& x) {
    if (x.length() > BitVec::MAX_N_BITS)
        throw runtime_error("Cannot convert NTL::vec_GF2 to a shorter BitVec");
    y = BitVec(x.rep[0]);
}

inline void conv(NTL::vec_GF2& y, const BitVec& x, int n_bits = -1) {
    if (n_bits > BitVec::MAX_N_BITS)
        throw runtime_error("Cannot convert BitVec to NTL::vec_GF2");
    if (n_bits == -1)
        n_bits = BitVec::MAX_N_BITS;
    y.SetLength(n_bits);
    y.rep[0] = x.mask(n_bits).get();
}

inline void conv(BitVector& y, const NTL::vec_GF2& x) {
    if (x.length() == 0) {
        y.resize(0);
        return;
    }
    assert(sizeof(word) == sizeof(x.rep[0]));
    y.resize(x.length());
    for (int i = 0; i < DIV_CEIL(x.length(), sizeof(word) * 8); i++)
        y.set_word(i, x.rep[i]);
}

inline void conv(NTL::vec_GF2& y, const BitVector& x) {
    if (x.size() == 0) {
        y.SetLength(0);
        return;
    }
    y.SetLength(x.size());
    assert(sizeof(word) == sizeof(y.rep[0]));
    for (int i = 0; i < DIV_CEIL(x.size(), sizeof(word) * 8); i++)
        y.rep[i] = x.get_word(i);
}


inline void pad(NTL::vec_GF2& x, int L) {
    if(x.length() >= L)
        return;
    x.SetLength(L);
}

#endif