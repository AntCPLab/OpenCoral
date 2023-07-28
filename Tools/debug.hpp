#ifndef TOOLS_DEBUG_HPP_
#define TOOLS_DEBUG_HPP_

#include "Tools/debug.h"

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

void print_gf2x_hex(const NTL::GF2X& x, const char* tag) {
    cout << hex << tag << ": " << x.xrep[0] << dec << endl;
}

void print_gf2e_hex(const NTL::GF2E& x, const char* tag) {
    print_gf2x_hex(x._GF2E__rep, tag);
}

void print_vecgf2_hex(const NTL::vec_GF2& x, const char* tag) {
    cout << hex << tag << ": " << x.rep[0] << dec << endl;
}

void print_total_comm(const Player& P, const char* tag) {
    auto comm_stats = P.total_comm();
    size_t rounds = 0;
    for (auto& x : comm_stats)
        rounds += x.second.rounds;
    std::cout << "[" << tag << "] Data sent = " << comm_stats.sent / 1e6 << " MB in ~" << rounds
        << " rounds (party " << P.my_num() << std::endl;

}

void print_general(const char* label, const char* tag) {
    cout << "[" << tag << "] " << label << endl;
}

template<class T>
void print_general(const char* label, const T& x, const char* tag) {
    cout << "[" << tag << "] " << label << ": " << x << endl;
}

template<class T1, class T2>
void print_general(const char* label1, const T1& x1, const char* label2, const T2& x2, const char* tag) {
    cout << "[" << tag << "] " << label1 << ": " << x1 << ", " << label2 << ": " << x2 << endl;
}

template<class T1, class T2, class T3>
void print_general(const char* label1, const T1& x1, 
    const char* label2, const T2& x2, 
    const char* label3, const T3& x3,
    const char* tag) {
    cout << "[" << tag << "] " << label1 << ": " << x1 
        << ", " << label2 << ": " << x2 
        << ", " << label3 << ": " << x3 << endl;
}

#endif