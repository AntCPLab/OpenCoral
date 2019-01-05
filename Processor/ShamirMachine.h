/*
 * ShamirMachine.h
 *
 */

#ifndef PROCESSOR_SHAMIRMACHINE_H_
#define PROCESSOR_SHAMIRMACHINE_H_

#include "Tools/ezOptionParser.h"

class ShamirMachine
{
    static ShamirMachine* singleton;

protected:
    ez::ezOptionParser opt;
    int nparties;

public:
    int threshold;

    static ShamirMachine& s();

    ShamirMachine(int argc, const char** argv);
};

template<template<class U> class T>
class ShamirMachineSpec : ShamirMachine
{
public:
    ShamirMachineSpec(int argc, const char** argv);
};

#endif /* PROCESSOR_SHAMIRMACHINE_H_ */
