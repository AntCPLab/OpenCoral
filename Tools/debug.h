
#ifndef TOOLS_DEBUG_H_
#define TOOLS_DEBUG_H_

#include "Networking/Player.h"
#include "NTL/GF2X.h"
#include "NTL/GF2E.h"

template<class T>
T reveal(Player* P, const T& share) {
    octetStream os;
    share.pack(os);
    P->exchange_relative(-1, os);
    T other_share;
    other_share.unpack(os);

    T revealed = share + other_share;

    return revealed;
}

void print_gf2x_hex(const NTL::GF2X& x, const char* tag="") {
    cout << hex << tag << ": " << x.xrep[0] << dec << endl;
}

void print_gf2e_hex(const NTL::GF2E& x, const char* tag="") {
    print_gf2x_hex(x._GF2E__rep, tag);
}

void print_vecgf2_hex(const NTL::vec_GF2& x, const char* tag="") {
    cout << hex << tag << ": " << x.rep[0] << dec << endl;
}


#endif