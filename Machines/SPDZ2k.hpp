/*
 * SPDZ2k.cpp
 *
 */

#ifndef MACHINES_SPDZ2K_HPP_
#define MACHINES_SPDZ2K_HPP_

#include "Protocols/Spdz2kShare.h"
#include "Protocols/Spdz2kPrep.h"
#include "Protocols/SPDZ2k.h"

#include "GC/TinySecret.h"
#include "GC/TinyMC.h"
#include "GC/TinierSecret.h"
#include "GC/VectorInput.h"

#include "Processor/Data_Files.hpp"
#include "Processor/Instruction.hpp"
#include "Processor/Machine.hpp"
#include "Protocols/MAC_Check.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Beaver.hpp"
#include "Protocols/Share.hpp"
#include "Math/Z2k.hpp"

#include "Protocols/MascotPrep.hpp"
#include "Protocols/Spdz2kPrep.hpp"

#include "GC/ShareParty.h"
#include "GC/ShareSecret.h"
#include "GC/Secret.hpp"
#include "GC/TinierSharePrep.h"
#include "GC/CcdPrep.h"

#include "GC/VectorProtocol.hpp"
#include "GC/TinierSharePrep.hpp"
#include "GC/ShareSecret.hpp"
#include "Protocols/ShuffleSacrifice.hpp"
#include "Protocols/MalRepRingPrep.hpp"
#include "GC/CcdPrep.hpp"
#include "GC/TinyPrep.hpp"

#include "Tools/debug.hpp"

#endif