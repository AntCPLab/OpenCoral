/*
 * Setup.h
 *
 */

#ifndef MATH_SETUP_H_
#define MATH_SETUP_H_

#include "Math/bigint.h"
#include "Tools/mkpath.h"

#include <iostream>
#include <fstream>
using namespace std;

#ifndef PREP_DIR
#define PREP_DIR "Player-Data/"
#endif

/*
 * Routines to create and read setup files for the finite fields
 */

// Create setup file for gfp and gf2n
template<class T>
void generate_prime_setup(string dir, int lgp);
void generate_online_setup(string dirname, bigint& p, int lgp);
void write_online_setup(string dirname, const bigint& p);

// Setup primes only
// Chooses a p of at least lgp bits
bigint SPDZ_Data_Setup_Primes(int lgp);
void SPDZ_Data_Setup_Primes(bigint& p,int lgp,int& idx,int& m);
void generate_prime(bigint& p, int lgp, int m);

template<class T>
string get_prep_sub_dir(string prep_dir, int nparties, int log2mod)
{
    string res = prep_dir + "/" + to_string(nparties) + "-" + T::type_short();
    if (T::clear::length() > 1)
        log2mod = T::clear::length();
    if (log2mod > 1)
        res += "-" + to_string(log2mod);
    res += "/";
    if (mkdir_p(res.c_str()) < 0)
        throw file_error(res);
    return res;
}

template<class T>
string get_prep_sub_dir(string prep_dir, int nparties)
{
    return get_prep_sub_dir<T>(prep_dir, nparties, T::clear::length());
}

template<class T>
string get_prep_sub_dir(int nparties)
{
    return get_prep_sub_dir<T>(PREP_DIR, nparties);
}

template<class T>
void generate_prime_setup(string dir, int nparties, int lgp)
{
    bigint p;
    generate_online_setup(get_prep_sub_dir<T>(dir, nparties, lgp), p, lgp);
}

void init_gf2n(int gf2ndegree);

#endif /* MATH_SETUP_H_ */
