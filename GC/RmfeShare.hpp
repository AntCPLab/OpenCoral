/*
 * RmfeShare.hpp
 *
 */

#ifndef GC_RMFESHARE_HPP_
#define GC_RMFESHARE_HPP_

#include "RmfeShare.h"



// NOTE: This constructor includes decoding.
bitvec_rmfe::bitvec_rmfe(const gf2n_rmfe& encoded) {
    NTL::GF2X ntl_tmp;
    BitVec decoded;
    conv(ntl_tmp, encoded);
    conv(decoded, Gf2RMFE::s().decode(ntl_tmp));
    super::operator=(decoded);
}

// NOTE: This constructor includes encoding.
gf2n_rmfe::gf2n_rmfe(const bitvec_rmfe& decoded) {
    NTL::vec_GF2 ntl_tmp;
    conv(ntl_tmp, decoded, bitvec_rmfe::DEFAULT_LENGTH);
    conv(*this, Gf2RMFE::s().encode(ntl_tmp));
}

void gf2n_rmfe::add(octetStream& o, int n) {
    typename super::internal_type b = 0;
    o.consume((octet*) &b, (length() + 7) / 8);
    a ^= b;
}

void gf2n_rmfe::pack(octetStream& o, int n) const {
    // Assuming little-endian, and length is a multiple of 8
    o.append((octet*) &a, (length() + 7) / 8);
}
void gf2n_rmfe::unpack(octetStream& o, int n) {
    o.consume((octet*) &a, (length() + 7) / 8);
    normalize();
}

template<>
void Square<gf2n_rmfe>::to(gf2n_rmfe& result, false_type)
{
    int128 sum;
    for (int i = 0; i < gf2n_rmfe::degree(); i++)
        sum ^= int128(rows[i].get()) << i;
    result = sum;
}


namespace GC {

ShareThread<RmfeSecret>& RmfeShare::get_party()
{
    return ShareThread<RmfeSecret>::s();
}

void RmfeShare::public_input(bool input)
{
    auto& party = get_party();
    *this = super::constant(input, party.P->my_num(),
            party.MC->get_alphai());
}

}

#endif /* GC_RMFEHARE_HPP_ */
