// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Machine.h
 *
 */

#ifndef GC_MACHINE_H_
#define GC_MACHINE_H_

#include <GC/FakeSecret.h>
#include "GC/Clear.h"
#include "GC/Memory.h"

#include "Math/Share.h"
#include "Math/gf2n.h"

#include "Processor/Machine.h"

#include <vector>
using namespace std;

namespace GC
{

template <class T> class Program;

template <class T>
class Machine : public ::BaseMachine
{
public:
    Memory<T> MS;
    Memory<Clear> MC;
    Memory<Integer> MI;
    Memory<typename T::DynamicType>& MD;

    Machine(Memory<typename T::DynamicType>& MD);
    ~Machine();

    void reset(const Program<T>& program);

    void start_timer() { timer[0].start(); }
    void stop_timer() { timer[0].stop(); }
    void reset_timer() { timer[0].reset(); }
};

} /* namespace GC */

#endif /* GC_MACHINE_H_ */
