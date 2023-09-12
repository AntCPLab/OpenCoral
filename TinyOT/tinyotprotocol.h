
#ifndef TINYOT_TINYOTPROTOCOL_H__
#define TINYOT_TINYOTPROTOCOL_H__

#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Protocols/Replicated.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "TinyOT/tinyotshare.h"
#include "TinyOT/tinyotmc.h"
#include "TinyOT/tinyotprep.h"


class TinyOTProtocol: public ProtocolBase<TinyOTShare> {
	typedef TinyOTShare T;
public:

	Player& P;

    TinyOTProtocol(Player& P) : P(P) {}

	template<class T>
	void init(Preprocessing<T>& prep, typename T::MAC_Check& MC) {}

	void init_mul() { throw runtime_error("init_mul not implemented"); }
	void prepare_mul(const T& x, const T& y, int n = -1) { throw runtime_error("prepare_mul not implemented"); }
	void exchange() { throw runtime_error("exchange not implemented"); }
	T finalize_mul(int n = -1) { throw runtime_error("finalize_mul not implemented"); }
};

#endif 