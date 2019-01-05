/*
 * ShamirMachine.cpp
 *
 */

#include "ShamirMachine.h"
#include "Math/ShamirShare.h"
#include "Math/MaliciousShamirShare.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "ReplicatedMachine.hpp"

ShamirMachine* ShamirMachine::singleton = 0;

ShamirMachine& ShamirMachine::s()
{
    if (singleton)
        return *singleton;
    else
        throw runtime_error("no singleton");
}

ShamirMachine::ShamirMachine(int argc, const char** argv)
{
    if (singleton)
        throw runtime_error("there can only be one");
    else
        singleton = this;

    opt.add(
            "3", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Number of players", // Help description.
            "-N", // Flag token.
            "--nparties" // Flag token.
    );
    opt.add(
            "", // Default.
            0, // Required?
            1, // Number of args expected.
            0, // Delimiter if expecting multiple args.
            "Number of corrupted parties (default: just below half)", // Help description.
            "-T", // Flag token.
            "--threshold" // Flag token.
    );
    opt.parse(argc, argv);
    opt.get("-N")->getInt(nparties);
    if (opt.isSet("-T"))
        opt.get("-T")->getInt(threshold);
    else
        threshold = (nparties - 1) / 2;
    cerr << "Using threshold " << threshold << " out of " << nparties << endl;
    if (2 * threshold >= nparties)
        throw runtime_error("threshold too high");
}

template<template<class U> class T>
ShamirMachineSpec<T>::ShamirMachineSpec(int argc, const char** argv) :
        ShamirMachine(argc, argv)
{
    ReplicatedMachine<T<gfp>, T<gf2n>>(argc, argv, "shamir", opt, nparties);
}

template class ShamirMachineSpec<ShamirShare>;
template class ShamirMachineSpec<MaliciousShamirShare>;
