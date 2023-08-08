
#ifndef TINYOT_TINYOTPROTOCOL_H__
#define TINYOT_TINYOTPROTOCOL_H__

#include "TinyOT/fpre.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "TinyOT/tinyotshare.h"
#include "TinyOT/tinyotmc.h"
#include "TinyOT/tinyotprep.h"

class TinyOTProtocol {
public:
	Player& P;

    TinyOTProtocol(Player& P) : P(P) {}

	template<class T>
	void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {}
};


#endif 