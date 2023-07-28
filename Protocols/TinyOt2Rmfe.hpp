#ifndef PROTOCOLS_TINYOT2RMFE_HPP_
#define PROTOCOLS_TINYOT2RMFE_HPP_

#include "./TinyOt2Rmfe.h"
#include "Tools/debug.h"
#include "Tools/performance.h"

using std::vector;
using GC::RmfeShare;
using GC::RmfeSecret;
using GC::ShareThread;


void TinyOt2Rmfe::convert(vector<RmfeShare>& rmfe_shares, const vector<TinyOTShare>& tinyot_shares) {
    if (tinyot_shares.size() % RmfeShare::default_length != 0)
        throw runtime_error("Input TinyOT batch size is not a multiple of RmfeShare packing size");
    const int s = (40 + RmfeShare::default_length - 1) / RmfeShare::default_length;
    int n = tinyot_shares.size() / RmfeShare::default_length;
    int l = RmfeShare::default_length;

    // Construct Rmfe Input
    auto& party = ShareThread<RmfeSecret>::s();
    auto& MC = party.MC->get_part_MC();
    auto& prep = party.DataF.get_part();
    auto& P = *party.P;
    RmfeShare::Input input(MC, prep, P);
    input.reset_all(P);

    time_log("random tiny ot");
    // Get random tiny ot shares and Input to RmfeShare
    vector<TinyOTShare> r(s * l);
    for(int i = 0; i < s; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            int k = i * l + j;
            tinyot_prep->get_random_abit(r[k].MAC, r[k].KEY);
            raw ^= ((long) r[k].get_bit().get()) << j;
        }
        input.add_from_all_decoded(BitVec(raw));
    }
    time_log("random tiny ot");

    time_log("input tiny ot");
    // Input param tiny ot shares to RmfeShare
    for(int i = 0; i < n; i++) {
        long raw = 0;
        for(int j = 0; j < l; j++) {
            raw ^= ((long) tinyot_shares[i*l + j].get_bit().get()) << j;
        }
        input.add_from_all_decoded(BitVec(raw));
    }
    time_log("input tiny ot");

    time_log("finalize input");
    input.exchange();
    // Obtain RmfeShare (possibly incorrect)
    vector<RmfeShare> y(n), q(s);
    for(int i = 0; i < s + n; i++) {
        if(i < s)
            q[i] = input.finalize(0) + input.finalize(1);
        else
            y[i - s] = input.finalize(0) + input.finalize(1);
    }
    time_log("finalize input");


    // [TODO] need to replace with coin tossing
    PRNG prng;
    prng.SetSeed((const unsigned char*) "insecure");

    time_log("linear comb");
    // Make a random linear combination
    vector<RmfeShare> z_share(s);
    vector<RmfeShare::open_type> z(s);
    vector<TinyOTShare> z_prime_share(s * l);
    vector<Bit> z_prime(s * l);
    for(int k = 0; k < s; k++) {
        z_share[k] = q[k];
        for(int j = 0; j < n; j++) {
            acc_time_log("prng");
            RmfeShare::raw_type b = prng.get<RmfeShare::raw_type>();
            acc_time_log("prng");
            z_share[k] += y[j] * RmfeShare::open_type(b);

            for(int h = 0; h < l; h++) {
                if (j == 0)
                    z_prime_share[k * l + h] = r[k * l + h];
                z_prime_share[k * l + h] += tinyot_shares[j * l + h] * b.get_bit(h);
            }
        }
    }
    time_log("linear comb");
    time_log("open");
    MC.POpen(z, z_share, P);
    tinyot_mc->POpen(z_prime, z_prime_share, P);
    time_log("open");

    time_log("consistency");
    // Consistency check
    for(int k = 0; k < s; k++) {
        RmfeShare::raw_type zk(z[k]);
        for(int h = 0; h < l; h++) {
            if(zk.get_bit(h) != z_prime[k*l + h].get())
                throw runtime_error("Inconsistency found between RMFE and TinyOT");
        }
    }
    time_log("consistency");
    print_profiling();

    rmfe_shares = std::move(y);
}

#endif