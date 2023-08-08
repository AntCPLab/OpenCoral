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