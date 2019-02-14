/*
 * ShamirMachine.cpp
 *
 */

#include <Machines/ShamirMachine.h>
#include "Math/ShamirShare.h"
#include "Math/MaliciousShamirShare.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "Processor/ReplicatedMachine.hpp"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"

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

template<>
Preprocessing<ShamirShare<gfp>>* Preprocessing<ShamirShare<gfp>>::get_live_prep(
   SubProcessor<ShamirShare<gfp>>* proc)
{
 return new ReplicatedPrep<ShamirShare<gfp>>(proc);
}

template<>
Preprocessing<ShamirShare<gf2n>>* Preprocessing<ShamirShare<gf2n>>::get_live_prep(
   SubProcessor<ShamirShare<gf2n>>* proc)
{
 return new ReplicatedPrep<ShamirShare<gf2n>>(proc);
}

template<>
Preprocessing<MaliciousShamirShare<gfp>>* Preprocessing<MaliciousShamirShare<gfp>>::get_live_prep(
   SubProcessor<MaliciousShamirShare<gfp>>* proc)
{
 (void) proc;
 return new MaliciousRepPrep<MaliciousShamirShare<gfp>>(proc);
}

template<>
Preprocessing<MaliciousShamirShare<gf2n>>* Preprocessing<MaliciousShamirShare<gf2n>>::get_live_prep(
   SubProcessor<MaliciousShamirShare<gf2n>>* proc)
{
 (void) proc;
 return new MaliciousRepPrep<MaliciousShamirShare<gf2n>>(proc);
}

template class ShamirMachineSpec<ShamirShare>;
template class ShamirMachineSpec<MaliciousShamirShare>;

template class Machine<ShamirShare<gfp>, ShamirShare<gf2n>>;
template class Machine<MaliciousShamirShare<gfp>, MaliciousShamirShare<gf2n>>;
