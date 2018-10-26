/*
 * Secret.cpp
 *
 */

#include "YaoGarbleWire.h"
#include "YaoEvalWire.h"

#include "GC/Machine.hpp"
#include "GC/Processor.hpp"
#include "GC/Secret.hpp"
#include "GC/Thread.hpp"
#include "GC/ThreadMaster.hpp"

namespace GC
{

template class Secret<YaoGarbleWire>;
template class Secret<YaoEvalWire>;

template void Secret<YaoGarbleWire>::reveal(Clear& x);
template void Secret<YaoEvalWire>::reveal(Clear& x);

template class Machine< Secret<YaoGarbleWire> >;
template class Machine< Secret<YaoEvalWire> >;

template class Processor< Secret<YaoGarbleWire> >;
template class Processor< Secret<YaoEvalWire> >;

template class Thread< Secret<YaoGarbleWire> >;
template class Thread< Secret<YaoEvalWire> >;

template class ThreadMaster< Secret<YaoGarbleWire> >;
template class ThreadMaster< Secret<YaoEvalWire> >;

}
