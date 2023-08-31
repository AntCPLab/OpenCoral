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
void reveal(Player* P, const edabitpack<T>& ep, const char* tag) {
    cout << "[" << tag << "] edabitpack A: ";
    for (size_t i = 0; i < ep.first.size(); i++) {
        cout << hex << (reveal(P, ep.first[i])).get_share() << " ";
    }
    cout << endl;
    cout << "[" << tag << "] edabitpack B: ";
    vector<typename T::bit_type::clear> b;
    for (size_t j = 0; j < ep.second.size(); j++)
        b.push_back(typename T::bit_type::clear(reveal(P, typename T::bit_type(ep.second[j])).get_share()));
    for (size_t i = 0; i < ep.first.size(); i++) {
        typename T::bit_type::clear x;
        for (size_t j = 0; j < ep.second.size(); j++)
            x += typename T::bit_type::clear(b[j].get_bit(i)) << j;
        cout << x << " ";
    }
    cout << endl;
    cout << "[" << tag << "] edabitpack B[" << dec << ep.second.size() << hex << "] raw: ";
    for (size_t j = 0; j < ep.second.size(); j++)
        cout << hex << typename T::bit_type::clear((reveal(P, ep.second[j])).get_share()) << " ";
    cout << dec << endl;
}

template<class T>
void reveal(Player* P, const dabitpack<T>& dp, const char* tag) {
    cout << "[" << tag << "] dabitpack A: ";
    for (size_t i = 0; i < dp.first.size(); i++) {
        cout << hex << (reveal(P, dp.first[i])).get_share() << " ";
    }
    cout << endl;
    cout << "[" << tag << "] dabitpack B: ";
    typename T::bit_type::clear b = typename T::bit_type::clear(reveal(P, typename T::bit_type::part_type(dp.second)).get_share());
    for (size_t j = 0; j < T::bit_type::default_length; j++)
        cout << b.get_bit(j) << " ";
    cout << dec << endl;
}

template<class T>
void print_general(const char* label, const T& x, const char* tag) {
#ifdef VERBOSE_DEBUG_PRINT
    cout << "[" << tag << "] " << label << ": " << x << endl;
#endif
}

template<class T1, class T2>
void print_general(const char* label1, const T1& x1, const char* label2, const T2& x2, const char* tag) {
#ifdef VERBOSE_DEBUG_PRINT
    cout << "[" << tag << "] " << label1 << ": " << x1 << ", " << label2 << ": " << x2 << endl;
#endif
}

template<class T1, class T2, class T3>
void print_general(const char* label1, const T1& x1, 
    const char* label2, const T2& x2, 
    const char* label3, const T3& x3,
    const char* tag) {
#ifdef VERBOSE_DEBUG_PRINT
    cout << "[" << tag << "] " << label1 << ": " << x1 
        << ", " << label2 << ": " << x2 
        << ", " << label3 << ": " << x3 << endl;
#endif
}

#endif