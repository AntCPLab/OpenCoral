/*
 * YaoEvalMaster.cpp
 *
 */

#include "YaoEvalMaster.h"
#include "YaoEvaluator.h"

YaoEvalMaster::YaoEvalMaster(bool continuous) : continuous(continuous)
{
}

Thread<Secret<YaoEvalWire>>* YaoEvalMaster::new_thread(int i)
{
    return new YaoEvaluator(i, *this);
}
