/*
 * YaoGarbleMaster.cpp
 *
 */

#include "YaoGarbleMaster.h"
#include "YaoGarbler.h"

YaoGarbleMaster::YaoGarbleMaster(bool continuous, OnlineOptions& opts, int threshold) :
        super(opts), continuous(continuous), threshold(threshold)
{
    PRNG G;
    G.ReSeed();
    delta = G.get_doubleword();
    delta.set_signal(1);
}

Thread<Secret<YaoGarbleWire>>* YaoGarbleMaster::new_thread(int i)
{
    return new YaoGarbler(i, *this);
}
