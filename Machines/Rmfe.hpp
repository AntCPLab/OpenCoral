#ifndef MACHINES_RMFE_HPP_
#define MACHINES_RMFE_HPP_

// There is some compiling problem when not including this header first.
// It might be a compiler bug for resolving macros and var names.
#include <emp-tool/emp-tool.h>

// #include "GC/RmfeSecret.h"
#include "GC/RmfeShare.h"
#include "GC/RmfeInput.hpp"
// #include "GC/RmfeVectorProtocol.hpp"
#include "GC/RmfeSharePrep.hpp"
#include "GC/RmfeShare.hpp"
#include "GC/VectorPrep.hpp"
#include "GC/Secret.hpp"
#include "GC/ShareSecret.hpp"
#include "GC/RmfeMultiplier.hpp"

#include "Protocols/Share.hpp"
#include "Protocols/fake-stuff.hpp"
#include "Protocols/Replicated.hpp"
#include "Protocols/MAC_Check_Base.hpp"
#include "Protocols/RmfeBeaver.hpp"
#include "Protocols/ReplicatedPrep.hpp"
#include "Protocols/ShuffleSacrifice.hpp"
#include "Protocols/MalRepRingPrep.hpp"

#include "Processor/Data_Files.hpp"

#include "Protocols/TinyOt2Rmfe.hpp"
#include "Protocols/RmfeShareConverter.hpp"

#include "OT/NPartyTripleGenerator.hpp"

#include "Math/Square.hpp"

#endif