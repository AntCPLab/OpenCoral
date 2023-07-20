#ifndef TOOLS_MPDZ_NTL_TYPES_H_
#define TOOLS_MPDZ_NTL_TYPES_H_

#include "Math/gf2n.h"
#include "Math/mfe.h"
#include "Math/BitVec.h"

void ntl_gf2x_to_mpdz_gf2n(gf2n_short& y, const NTL::GF2X& x) {
    if (deg(x) >= gf2n_short::degree())
        throw runtime_error("Cannot convert NTL::GF2X to a smaller gf2n element");
    // empty poly returns deg(*)=-1, but it actually means 0
    if (deg(x) == -1)
        y = gf2n_short(0);
    else
        y = gf2n_short(x.xrep[0]);
}

void conv(gf2n_short& y, const NTL::GF2X& x) {
    ntl_gf2x_to_mpdz_gf2n(y, x);
}

gf2n_short ntl_gf2x_to_mpdz_gf2n(const NTL::GF2X& x) {
    gf2n_short y;
    ntl_gf2x_to_mpdz_gf2n(y, x);
    return y;
}

void mpdz_gf2n_to_ntl_gf2x(NTL::GF2X& y, const gf2n_short& x) {
    if (x.get_word() == 0) {
        clear(y);
    }
    else {
        y.xrep.SetLength(1);
        y.xrep[0] = x.get_word();
    }
}

void conv(NTL::GF2X& y, const gf2n_short& x) {
    mpdz_gf2n_to_ntl_gf2x(y, x);
}

NTL::GF2X mpdz_gf2n_to_ntl_gf2x(const gf2n_short& x) {
    NTL::GF2X y;
    mpdz_gf2n_to_ntl_gf2x(y, x);
    return y;
}

void conv(BitVec& y, const NTL::vec_GF2& x) {
    if (x.length() > BitVec::MAX_N_BITS)
        throw runtime_error("Cannot convert NTL::vec_GF2 to a shorter BitVec");
    y = BitVec(x.rep[0]);
}

void conv(NTL::vec_GF2& y, const BitVec& x, int n_bits = -1) {
    if (n_bits > BitVec::MAX_N_BITS)
        throw runtime_error("Cannot convert BitVec to NTL::vec_GF2");
    if (n_bits == -1)
        n_bits = BitVec::MAX_N_BITS;
    y.SetLength(n_bits);
    y.rep[0] = x.mask(n_bits).get();
}

void pad(NTL::vec_GF2& x, int L) {
    if(x.length() >= L)
        return;
    x.SetLength(L);
}

#endif