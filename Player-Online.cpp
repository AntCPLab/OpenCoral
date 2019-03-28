/*
 * Player-Online.cpp
 *
 */

#include "Processor/config.h"

#include "Player-Online.hpp"

int main(int argc, const char** argv)
{
    ez::ezOptionParser opt;
    return spdz_main<sgfp>(argc, argv, opt);
}
