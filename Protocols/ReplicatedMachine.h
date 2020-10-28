/*
 * ReplicatedMachine.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDMACHINE_H_
#define PROTOCOLS_REPLICATEDMACHINE_H_

#include <string>
using namespace std;

#include "Tools/ezOptionParser.h"
#include "Processor/OnlineOptions.h"

template<class T, class U>
class ReplicatedMachine
{
public:
    ReplicatedMachine(int argc, const char **argv, ez::ezOptionParser &opt,
            OnlineOptions &online_opts, int n_players = 3);
    ReplicatedMachine(int argc, const char** argv, string name,
            ez::ezOptionParser& opt, int nplayers = 3);
    ReplicatedMachine(int argc, const char** argv, ez::ezOptionParser& opt,
            int nplayers = 3) :
            ReplicatedMachine(argc, argv, "", opt, nplayers)
    {
    }
};

template<template<class T> class U>
class ReplicatedFieldMachine
{
public:
    ReplicatedFieldMachine(int argc, const char** argv);
};

#endif /* PROTOCOLS_REPLICATEDMACHINE_H_ */
