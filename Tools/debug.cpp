
#include "Tools/debug.h"

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
