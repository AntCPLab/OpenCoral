#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/Share.hpp"

#include "Protocols/MascotPrep.hpp"

#include "GC/TinierSecret.h"
#include "GC/TinyMC.h"
#include "GC/TinierPrep.h"

#include "GC/ShareParty.hpp"
#include "GC/Secret.hpp"
#include "GC/TinyPrep.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/TinierSharePrep.hpp"

template class Machine<Share<gfp>, Share<gf2n>>;
