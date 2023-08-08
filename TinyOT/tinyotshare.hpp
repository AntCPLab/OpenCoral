
#ifndef TINYOT_TINYOTSHARE_HPP__
#define TINYOT_TINYOTSHARE_HPP__

#include "TinyOT/fpre.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "tinyotmc.h"
#include "tinyotshare.h"


TinyOTShare::MC* TinyOTShare::new_mc(mac_key_type key) {
	return new MC(key);
}


#endif 