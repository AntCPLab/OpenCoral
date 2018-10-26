/*
 * Program.cpp
 *
 */

#include "YaoEvalWire.h"
#include "YaoGarbleWire.h"

#include "GC/Instruction.hpp"
#include "GC/Program.hpp"

namespace GC
{

template class Instruction< Secret<YaoGarbleWire> >;
template class Instruction< Secret<YaoEvalWire> >;

template class Program< Secret<YaoGarbleWire> >;
template class Program< Secret<YaoEvalWire> >;

}
