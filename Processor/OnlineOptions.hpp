/*
 * OnlineOptions.hpp
 *
 */

#ifndef PROCESSOR_ONLINEOPTIONS_HPP_
#define PROCESSOR_ONLINEOPTIONS_HPP_

#include "OnlineOptions.h"

template<class T>
OnlineOptions::OnlineOptions(ez::ezOptionParser& opt, int argc,
        const char** argv, T, bool default_live_prep) :
        OnlineOptions(opt, argc, argv, T::dishonest_majority ? 1000 : 0,
                default_live_prep, T::clear::prime_field)
{
    if (T::has_trunc_pr)
        opt.add(
                to_string(trunc_error).c_str(), // Default.
                0, // Required?
                1, // Number of args expected.
                0, // Delimiter if expecting multiple args.
                "Probabilistic truncation error "
                "(2^-x, default: 40)", // Help description.
                "-E", // Flag token.
                "--trunc-error" // Flag token.
        );
}

#endif /* PROCESSOR_ONLINEOPTIONS_HPP_ */
