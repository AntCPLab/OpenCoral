/*
 * Semi.cpp
 *
 */

#include "Math/SemiShare.h"
#include "Math/Semi2kShare.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Auth/SemiMC.h"
#include "Processor/SemiPrep.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Processor/MascotPrep.hpp"
#include "Processor/SemiPrep.hpp"
#include "Processor/SemiInput.hpp"
#include "Auth/MAC_Check.hpp"
#include "Auth/fake-stuff.hpp"
#include "Auth/SemiMC.hpp"

template class Machine<SemiShare<gfp>, SemiShare<gf2n>>;
template class Machine<Semi2kShare<64>, SemiShare<gf2n>>;
template class Machine<Semi2kShare<72>, SemiShare<gf2n>>;
