/*
 * Secret.cpp
 *
 */

#include "GC/Machine.hpp"
#include "GC/Processor.hpp"
#include "GC/Secret.hpp"
#include "GC/Thread.hpp"
#include "GC/ThreadMaster.hpp"

namespace GC
{

template class Secret<EvalRegister>;
template class Secret<PRFRegister>;
template class Secret<GarbleRegister>;
template class Secret<RandomRegister>;

template void Secret<EvalRegister>::reveal(Clear& x);
template void Secret<PRFRegister>::reveal(Clear& x);
template void Secret<GarbleRegister>::reveal(Clear& x);
template void Secret<RandomRegister>::reveal(Clear& x);

template class Machine< Secret<PRFRegister> >;
template class Machine< Secret<EvalRegister> >;
template class Machine< Secret<GarbleRegister> >;
template class Machine< Secret<RandomRegister> >;

template class Processor< Secret<PRFRegister> >;
template class Processor< Secret<EvalRegister> >;
template class Processor< Secret<GarbleRegister> >;
template class Processor< Secret<RandomRegister> >;

template class ThreadMaster< Secret<PRFRegister> >;
template class ThreadMaster< Secret<EvalRegister> >;
template class ThreadMaster< Secret<GarbleRegister> >;
template class ThreadMaster< Secret<RandomRegister> >;

}
