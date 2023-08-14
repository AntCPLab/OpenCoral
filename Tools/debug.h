
#ifndef TOOLS_DEBUG_H_
#define TOOLS_DEBUG_H_

#include "Networking/Player.h"
#include "NTL/GF2X.h"
#include "NTL/GF2E.h"

template<class T>
T reveal(Player* P, const T& share);

void print_gf2x_hex(const NTL::GF2X& x, const char* tag="");

void print_gf2e_hex(const NTL::GF2E& x, const char* tag="");

void print_vecgf2_hex(const NTL::vec_GF2& x, const char* tag="");

void print_total_comm(const Player& P, const char* tag="");

void print_general(const char* label, const char* tag="");

template<class T>
void print_general(const char* label, const T& x, const char* tag="");

template<class T1, class T2>
void print_general(const char* label1, const T1& x1, const char* label2, const T2& x2, const char* tag="");

template<class T1, class T2, class T3>
void print_general(const char* label1, const T1& x1, 
    const char* label2, const T2& x2, 
    const char* label3, const T3& x3,
    const char* tag="");


#endif