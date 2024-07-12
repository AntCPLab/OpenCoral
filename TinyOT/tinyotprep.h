
#ifndef TINYOT_TINYOTPREP_H__
#define TINYOT_TINYOTPREP_H__


#ifdef USE_SILENT_OT
#include "TinyOT/fpre_silent.h"
#include "Processor/BaseMachine.h"
#else
#include "TinyOT/fpre.h"
#endif
#include "Networking/EmpChannel.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "TinyOT/tinyotshare.h"
#include "Protocols/ReplicatedPrep.h"

class BufferTinyOTPrep : public Preprocessing<TinyOTShare> {

	EmpChannel *io;
	emp::Fpre<EmpChannel>* fpre;
	int triple_buf_idx;
	vector<emp::block> random_abit_MACs;
	vector<emp::block> random_abit_KEYs;

	Player* P;
	TwoPartyPlayer* player_2pc;
	int batch_size;

public:

	BufferTinyOTPrep(DataPositions& usage, int batch_size = 1000);

	void set_protocol(TinyOTShare::Protocol& protocol);

	void set_batch_size(int batch_size);
	int get_batch_size() {
		return fpre->batch_size;
	}
	void get_random_abit(emp::block& MAC, emp::block& KEY);

	TinyOTShare get_bit();

	void get_tinyot_triple(emp::block& aMAC, emp::block& aKEY,
		emp::block& bMAC, emp::block& bKEY,
		emp::block& cMAC, emp::block& cKEY);
	~BufferTinyOTPrep();
};

#endif 