#include "Math/Spdz2kShare.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Auth/MAC_Check.hpp"
#include "Auth/fake-stuff.hpp"

#include "Processor/MascotPrep.hpp"
#include "Processor/Spdz2kPrep.hpp"

template class Machine<sgfp, Share<gf2n>>;

template class Machine<Spdz2kShare<64, 64>, Share<gf2n>>;
template class Machine<Spdz2kShare<64, 48>, Share<gf2n>>;
