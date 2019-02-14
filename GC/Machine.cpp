/*
 * Machine.cpp
 *
 */

#include "MaliciousRepSecret.h"
#include "Auth/ReplicatedMC.h"
#include "Auth/MaliciousRepMC.h"

#include "Instruction.hpp"
#include "Machine.hpp"
#include "Processor.hpp"
#include "Program.hpp"
#include "Thread.hpp"
#include "ThreadMaster.hpp"

#include "Processor/Machine.hpp"
#include "Processor/Instruction.hpp"

namespace GC
{

extern template class ReplicatedSecret<SemiHonestRepSecret>;
extern template class ReplicatedSecret<MaliciousRepSecret>;

#define GC_MACHINE(T) \
    template class Instruction<T>; \
    template class Machine<T>; \
    template class Processor<T>; \
    template class Program<T>; \
    template class Thread<T>; \
    template class ThreadMaster<T>; \

GC_MACHINE(FakeSecret);
GC_MACHINE(SemiHonestRepSecret);
GC_MACHINE(MaliciousRepSecret)

} /* namespace GC */
