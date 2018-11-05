/*
 * YaoCommon.h
 *
 */

#ifndef YAO_YAOCOMMON_H_
#define YAO_YAOCOMMON_H_

#include <stdexcept>

class YaoCommon
{
    int log_n_threads;

public:
    static const int DONE = -1;
    static const int MORE = -2;

    long counter;

    YaoCommon() :
        log_n_threads(8), counter(0)
    {
    }

    void set_n_program_threads(int n_threads)
    {
        log_n_threads = floor(log2(n_threads));
    }

    long gate_id(long thread_num)
    {
        return counter + (thread_num << (64 - log_n_threads));
    }
};

#endif /* YAO_YAOCOMMON_H_ */
