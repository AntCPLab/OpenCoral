/*
 * ReplicatedPrivateOutput.cpp
 *
 */

#include "ReplicatedPrivateOutput.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"

template<class T>
void ReplicatedPrivateOutput<T>::start(int player, int target,
        int source)
{
    (void)player, (void)target, (void)source;
}

template<class T>
void ReplicatedPrivateOutput<T>::stop(int player, int source)
{
    (void)player, (void)source;
}

template class ReplicatedPrivateOutput<FixedVec<Integer, 2> >;
