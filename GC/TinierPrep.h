/*
 * TinierPrep.h
 *
 */

#ifndef GC_TINIERPREP_H_
#define GC_TINIERPREP_H_

#include "TinyPrep.h"

namespace GC
{

template<class T>
class TinierPrep : public TinyPrep<T>
{
public:
    TinierPrep(DataPositions& usage, ShareThread<T>& thread) :
            TinyPrep<T>(usage, thread)
    {
    }

    void buffer_inputs(int player)
    {
        this->buffer_inputs_(player, this->triple_generator);
    }
};

}

#endif /* GC_TINIERPREP_H_ */
