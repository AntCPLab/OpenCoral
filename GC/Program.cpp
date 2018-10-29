/*
 * Program.cpp
 *
 */

#include "Instruction.hpp"
#include "Program.hpp"

namespace GC
{

template class Instruction<FakeSecret>;
template class Instruction<ReplicatedSecret>;

template class Program<FakeSecret>;
template class Program<ReplicatedSecret>;

} /* namespace GC */
