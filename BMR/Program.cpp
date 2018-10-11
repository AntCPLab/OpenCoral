/*
 * Program.cpp
 *
 */

#include "GC/Instruction.hpp"
#include "GC/Program.hpp"

namespace GC
{

template class Instruction< Secret<PRFRegister> >;
template class Instruction< Secret<EvalRegister> >;
template class Instruction< Secret<GarbleRegister> >;
template class Instruction< Secret<RandomRegister> >;

template class Program< Secret<PRFRegister> >;
template class Program< Secret<EvalRegister> >;
template class Program< Secret<GarbleRegister> >;
template class Program< Secret<RandomRegister> >;

}
