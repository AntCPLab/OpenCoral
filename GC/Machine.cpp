/*
 * Machine.cpp
 *
 */

#include "MaliciousRepSecret.h"
#include "Protocols/ReplicatedMC.h"
#include "Protocols/MaliciousRepMC.h"

#include "Instruction.hpp"
#include "Machine.hpp"
#include "Processor.hpp"
#include "Program.hpp"
#include "Thread.hpp"
#include "ThreadMaster.hpp"

#include "Processor/Machine.hpp"
#include "Processor/Instruction.hpp"
#include "Protocols/MaliciousRepMC.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/Beaver.hpp"

namespace GC
{

#define GC_MACHINE(T) \
    template class Instruction<T>; \
    template class Machine<T>; \
    template class Processor<T>; \
    template class Program<T>; \
    template class Thread<T>; \
    template class ThreadMaster<T>; \

} /* namespace GC */
