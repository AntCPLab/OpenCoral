#ifndef PROTOCOLS_RMFESHARECONVERTER_HPP_
#define PROTOCOLS_RMFESHARECONVERTER_HPP_

#include "./RmfeShareConverter.h"
#include "Tools/debug.h"
#include "Tools/performance.h"

using std::vector;
using GC::RmfeShare;
using GC::ShareThread;


template<class T>
void RmfeShareConverter<T>::convert(vector<RmfeShare>& rmfe_shares, const vector<T>& src_shares) {
    if (src_shares.size() % RmfeShare::default_length != 0)
        throw runtime_error("Input share batch size is not a multiple of RmfeShare packing size");
    const int s = (40 + RmfeShare::default_length - 1) / RmfeShare::default_length;
    int n = src_shares.size() / RmfeShare::default_length;
    int l = RmfeShare::default_length;

    // Construct Rmfe Input
    auto& party = ShareThread<RmfeShare::whole_type>::s();
    // [zico] Why create a new mc here? Usually we cannot directly use the MC from ShareThread because
    // our outer application will probaly use it to open values. If we use it here, and convert is used
    // in the middle of outer openning, then the MC open inside convert will destroy the outer openning.
    // It is a general problem for nested openning with the same MC.
    // We use ShareThread for src_mc above because that mc is only expected to be used here. If we intend to
    // use it in outer application, then the application code should be carefully designed to not introduce the nested
    // openning for src_mc.
    auto dst_mc = GC::RmfeShare::new_mc(
            GC::ShareThread<GC::RmfeShare::whole_type>::s().MC->get_alphai());
    auto& prep = party.DataF.get_part();
    auto& P = *party.P;
    RmfeShare::Input input(*dst_mc, prep, P);
    input.reset_all(P);

    // Get random tiny ot shares and Input to RmfeShare
    vector<T> r(s * l);
    for(int i = 0; i < s; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            int k = i * l + j;
            // tinyot_prep->get_random_abit(r[k].MAC, r[k].KEY);
            // raw ^= ((long) r[k].get_bit().get()) << j;
            r[k] = src_prep->get_bit();
            raw ^= ((long) typename T::clear(r[k].get_bit(0).get_share()).get_bit(0)) << j;
        }
        // input.add_from_all_decoded(BitVec(raw));
        input.add_from_all(BitVec(raw));
    }

    // Input param tiny ot shares to RmfeShare
    for(int i = 0; i < n; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            raw ^= ((long) typename T::clear(src_shares[i*l + j].get_bit(0).get_share()).get_bit(0)) << j;
        }
        // input.add_from_all_decoded(BitVec(raw));
        input.add_from_all(BitVec(raw));
    }

    input.exchange();
    // Obtain RmfeShare (possibly incorrect)
    vector<RmfeShare> y(n), q(s);
    for(int i = 0; i < s + n; i++) {
        if(i < s)
            q[i] = input.finalize(0) + input.finalize(1);
        else
            y[i - s] = input.finalize(0) + input.finalize(1);
    }

    GlobalPRNG prng(P);

    // Make a random linear combination
    vector<RmfeShare> z_share(s);
    vector<RmfeShare::open_type> z(s);
    vector<T> z_prime_share(s * l);
    vector<typename T::open_type> z_prime(s * l);
    for(int k = 0; k < s; k++) {
        z_share[k] = q[k];
        for(int j = 0; j < n; j++) {
            RmfeShare::raw_type b = prng.get<RmfeShare::raw_type>();
            z_share[k] += y[j] * RmfeShare::open_type(b);

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
        RmfeShare::raw_type zk(z[k]);
        for(int h = 0; h < l; h++) {
            if(zk.get_bit(h) != z_prime[k*l + h].get_bit(0))
                throw runtime_error("Inconsistency found between RMFE and " + T::type_string());
        }
    }
    src_mc->Check(P);
    dst_mc->Check(P);
    delete dst_mc;

    rmfe_shares = std::move(y);
}

#endif