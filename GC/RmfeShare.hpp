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

gf2n_rmfe gf2n_rmfe::tau(const gf2n_rmfe& x) {
    return gf2n_rmfe(bitvec_rmfe(x));
}

bool gf2n_rmfe::is_normal() {
    return tau(*this) == *this;
}

template<>
void Square<gf2n_rmfe>::to(gf2n_rmfe& result, false_type)
{
    int128 sum;
    for (int i = 0; i < gf2n_rmfe::degree(); i++)
        sum ^= int128(rows[i].get()) << i;
    result = gf2n_rmfe(sum);
}


namespace GC {

ShareThread<RmfeShare::whole_type>& RmfeShare::get_party() {
    return ShareThread<RmfeShare::whole_type>::s();
}

void RmfeShare::public_input(long input) {
    auto& party = get_party();
    *this = super::constant(RmfeShare::open_type(RmfeShare::clear(input)), party.P->my_num(),
            party.MC->get_alphai());
}

void RmfeShare::load_clear(int n, const Integer& x) {
    if (n > default_length)
        throw out_of_range("loaded bit length (" + to_string(n) + ") too long for RmfeShare (" + to_string(default_length) + ")");
    if ((unsigned)n < 8 * sizeof(x) and (unsigned long) abs(x.get()) > (1ul << n))
        throw out_of_range("public value too long");
    public_input(x.get());
}

void RmfeShare::xor_(int n, const RmfeShare& x, const RmfeShare& y) {
    if (n != default_length)
        throw runtime_error("Invalid length for xor_ of RmfeShare: n = " + to_string(n));
    *this = (x ^ y); 
}

void RmfeShare::invert(int n, const This& x) {
    if (n != default_length)
        throw runtime_error("Invalid length for invert of RmfeShare: n = " + to_string(n));
    RmfeShare ones;
    ones.load_clear(default_length, -1);
    *this = RmfeShare(x + ones);
}

RmfeShare RmfeShare::operator&(const BitVec::super& other) const
{
    /** Constant multiplication requires one round of communication.
     * So calling this for each invididual mult constant is not recommended.
     * Better use the `andm` instruction with several mult constant together.
    */
    // auto& thread = ShareThread<RmfeShare>::s();
    // auto& protocol = thread.protocol;
    // protocol->init_mul_constant();
    // protocol->prepare_mul_constant(*this, RmfeShare::clear(other));
    // protocol->exchange_mul_constant();
    // return protocol->finalize_mul_constant();
    throw runtime_error("unexpected call to operator&");
}


template<class U>
void RmfeShare::trans(Processor<U>& processor, int n_outputs, const vector<int>& args)
{
    throw runtime_error("No trans for RmfeShare because it would require breaking down into bits, which would destroy MAC \
        on the the bit pack.");
}

void RmfeShare::andm(Processor<RmfeShare>& processor, const BaseInstruction& instruction) {
    assert(instruction.get_n() % default_length == 0);
    auto& thread = ShareThread<RmfeShare>::s();
    auto& protocol = thread.protocol;
    protocol->init_mul_constant();
    for (int i = 0; i < DIV_CEIL(instruction.get_n(), default_length); i++) {
        protocol->prepare_mul_constant(
            processor.S[instruction.get_r(1) + i], 
            RmfeShare::clear(processor.C[instruction.get_r(2) + i]));
    }
    protocol->exchange_mul_constant();
    for (int i = 0; i < DIV_CEIL(instruction.get_n(), default_length); i++) {
        processor.S[instruction.get_r(0) + i] = protocol->finalize_mul_constant();
    }
}

}


#endif /* GC_RMFEHARE_HPP_ */
