#ifndef PROTOCOLS_GENERALSHARECONVERTER_HPP_
#define PROTOCOLS_GENERALSHARECONVERTER_HPP_

#include "./GeneralShareConverter.h"
#include "Tools/debug.h"
#include "Tools/performance.h"

using std::vector;
using GC::ShareThread;


template<class SrcType, class DstType>
void GeneralShareConverter<SrcType, DstType>::convert(vector<DstType>& dst_shares, const vector<SrcType>& src_shares) {
    // if (src_shares.size() % RmfeShare::default_length != 0)
    //     throw runtime_error("Input share batch size is not a multiple of RmfeShare packing size");
    const int s = (40 + DstType::default_length - 1) / DstType::default_length;
    int n = src_shares.size() / DstType::default_length;
    int l = DstType::default_length;

    // Construct Dst Input
    auto& party = ShareThread<typename DstType::whole_type>::s();
    // [zico] Why create a new mc here? Usually we cannot directly use the MC from ShareThread because
    // our outer application will probaly use it to open values. If we use it here, and convert is used
    // in the middle of outer openning, then the MC open inside convert will destroy the outer openning.
    // It is a general problem for nested openning with the same MC.
    // We use ShareThread for src_mc above because that mc is only expected to be used here. If we intend to
    // use it in outer application, then the application code should be carefully designed to not introduce the nested
    // openning for src_mc.
    auto dst_mc = DstType::new_mc(
            GC::ShareThread<typename DstType::whole_type>::s().MC->get_alphai());
    auto& prep = party.DataF.get_part();
    auto& P = *party.P;
    typename DstType::Input input(*dst_mc, prep, P);
    input.reset_all(P);

    // Get random SrcType shares and Input to DstType
    vector<SrcType> r(s * l);
    for(int i = 0; i < s; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            int k = i * l + j;
            r[k] = src_prep->get_bit();
            raw ^= ((long) typename SrcType::clear(r[k].get_bit(0).get_share()).get_bit(0)) << j;
        }
        input.add_from_all(typename DstType::open_type(raw));
    }

    // Input param SrcType shares to DstType
    for(int i = 0; i < n; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            raw ^= ((long) typename SrcType::clear(src_shares[i*l + j].get_bit(0).get_share()).get_bit(0)) << j;
        }
        input.add_from_all(typename DstType::open_type(raw));
    }

    input.exchange();
    // Obtain DstType (possibly incorrect)
    vector<DstType> y(n), q(s);
    for(int i = 0; i < s + n; i++) {
        if(i < s)
            q[i] = input.finalize(0) + input.finalize(1);
        else
            y[i - s] = input.finalize(0) + input.finalize(1);
    }

    GlobalPRNG prng(P);

    // Make a random linear combination
    vector<DstType> z_share(s);
    vector<typename DstType::open_type> z(s);
    vector<SrcType> z_prime_share(s * l);
    vector<typename SrcType::open_type> z_prime(s * l);
    for(int k = 0; k < s; k++) {
        z_share[k] = q[k];
        for(int j = 0; j < n; j++) {
            typename DstType::clear b = prng.get<typename DstType::clear>();
            z_share[k] += y[j] * typename DstType::open_type(b);

            for(int h = 0; h < l; h++) {
                if (j == 0)
                    z_prime_share[k * l + h] = r[k * l + h];
                z_prime_share[k * l + h] += src_shares[j * l + h] * b.get_bit(h);
            }
        }
    }
    dst_mc->POpen(z, z_share, P);
    src_mc->POpen(z_prime, z_prime_share, P);

    // Consistency check
    for(int k = 0; k < s; k++) {
        typename DstType::clear zk(z[k]);
        for(int h = 0; h < l; h++) {
            if(zk.get_bit(h) != z_prime[k*l + h].get_bit(0))
                throw runtime_error("Inconsistency found between DstType and SrcType");
        }
    }

    dst_mc->Check(P);
    delete dst_mc;

    dst_shares = std::move(y);
}

#endif