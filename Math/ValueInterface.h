/*
 * ValueInterface.h
 *
 */

#ifndef MATH_VALUEINTERFACE_H_
#define MATH_VALUEINTERFACE_H_

#include "Exceptions/Exceptions.h"

class OnlineOptions;
class bigint;

class ValueInterface
{
public:
    static const int MAX_EDABITS = 0;

    template<class T>
    static void init(bool mont = true) { (void) mont; }
    static void init_default(int l) { (void) l; }
    static void init_field() {}

    static void read_or_generate_setup(const string&, const OnlineOptions&) {}
    template<class T>
    static void generate_setup(string, int, int) {}

    static bigint pr() { throw runtime_error("no prime modulus"); }

    static int power_of_two(bool, int) { throw not_implemented(); }

    void normalize() {}
};

#endif /* MATH_VALUEINTERFACE_H_ */
