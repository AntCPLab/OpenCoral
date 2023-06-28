
#include "Math/gf2n.h"
#include "Math/mfe.h"
#include "Math/BitVec.h"

void conv(gf2n_short& y, const NTL::GF2X& x) {
    ntl_gf2x_to_mpdz_gf2n(y, x);
}

void ntl_gf2x_to_mpdz_gf2n(gf2n_short& y, const NTL::GF2X& x) {
    if (deg(x) >= gf2n_short::degree())
        throw runtime_error("Cannot convert NTL::GF2X to a smaller gf2n element");
    y = gf2n_short(x.xrep[0]);
}

gf2n_short ntl_gf2x_to_mpdz_gf2n(const NTL::GF2X& x) {
    gf2n_short y;
    ntl_gf2x_to_mpdz_gf2n(y, x);
    return y;
}

void conv(NTL::GF2X& y, const gf2n_short& x) {
    mpdz_gf2n_to_ntl_gf2x(y, x);
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