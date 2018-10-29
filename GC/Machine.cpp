/*
 * Machine.cpp
 *
 */

#include "Machine.hpp"
#include "Processor.hpp"
#include "Thread.hpp"
#include "ThreadMaster.hpp"

namespace GC
{

template class Machine<FakeSecret>;
template class Machine<ReplicatedSecret>;

template class Processor<FakeSecret>;
template class Processor<ReplicatedSecret>;

template class Thread<FakeSecret>;
template class Thread<ReplicatedSecret>;

template class ThreadMaster<FakeSecret>;
template class ThreadMaster<ReplicatedSecret>;

} /* namespace GC */
