/*
 * ReplicatedFieldMachine.hpp
 *
 */

#ifndef PROTOCOLS_REPLICATEDFIELDMACHINE_HPP_
#define PROTOCOLS_REPLICATEDFIELDMACHINE_HPP_

#include "ReplicatedMachine.hpp"

template<template<class U> class T>
ReplicatedFieldMachine<T>::ReplicatedFieldMachine(int argc,
        const char **argv)
{
    ez::ezOptionParser opt;
    OnlineOptions online_opts(opt, argc, argv, 0, true, true);
    int n_limbs = DIV_CEIL(online_opts.lgp, 64);
    switch (n_limbs)
    {
#undef X
#define X(L) \
    case L: \
        ReplicatedMachine<T<gfp_<0, L>>, T<gf2n>>(argc, argv, opt, online_opts); \
        break;
#ifdef MORE_PRIMES
    X(1) X(2) X(3)
#endif
#if GFP_MOD_SZ > 3 or not defined(MORE_PRIMES)
    X(GFP_MOD_SZ)
#endif
#undef X
    default:
        cerr << "Not compiled for " << online_opts.lgp << "-bit primes" << endl;
        cerr << "Compile with -DGFP_MOD_SZ=" << n_limbs << endl;
        exit(1);
    }
}

#endif /* PROTOCOLS_REPLICATEDFIELDMACHINE_HPP_ */
