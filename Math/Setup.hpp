/*
 * Setup.hpp
 *
 */

#ifndef MATH_SETUP_HPP_
#define MATH_SETUP_HPP_

#include "gfp.h"

template<class T = gfp>
void read_setup(const string& dir_prefix, int lgp = -1)
{
    bigint p;

    string filename = dir_prefix + "Params-Data";

    if (dir_prefix.compare("") == 0)
        filename = string(PREP_DIR "Params-Data");

#ifdef DEBUG_FILES
  cerr << "loading params from: " << filename << endl;
#endif
    ifstream inpf(filename.c_str());
    inpf >> p;
    if (inpf.fail())
    {
        if (lgp > 0)
        {
            cerr << "No modulus found in " << filename << ", generating " << lgp
                    << "-bit prime" << endl;
            T::init_default(lgp);
        }
        else
            throw file_error(filename.c_str());
    }
    else
        T::init_field(p);

    inpf.close();
}

#endif /* MATH_SETUP_HPP_ */
