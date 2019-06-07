#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/Share.hpp"

#include "Protocols/MascotPrep.hpp"

template class Machine<Share<gfp>, Share<gf2n>>;
