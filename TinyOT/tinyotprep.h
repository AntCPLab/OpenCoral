
#ifndef TINYOT_TINYOTPREP_H__
#define TINYOT_TINYOTPREP_H__

#include "TinyOT/fpre.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "TinyOT/tinyotshare.h"

class BufferTinyOTPrep : public Preprocessing<TinyOTShare> {

	emp::NetIO *io;
	emp::Fpre<emp::NetIO>* fpre;
	int triple_buf_idx;
	vector<emp::block> random_abit_MACs;
	vector<emp::block> random_abit_KEYs;

	Player* P;
	int port;
	int batch_size;

public:

	static atomic<int> port_resource;
	static int get_next_available_port();

	// BufferTinyOTPrep(int party, int port = 12345, int batch_size = 1000) {
	// 	io = new emp::NetIO(party==emp::ALICE ? nullptr:emp::IP, port);
	// 	fpre = new emp::Fpre<emp::NetIO>(io, party, batch_size);
	// 	triple_buf_idx = batch_size;

	// 	random_abit_MACs.reserve(fpre->batch_size);
	// 	random_abit_KEYs.reserve(fpre->batch_size);
	// }

	BufferTinyOTPrep(DataPositions& usage, int port = -1, int batch_size = 1000);

	void set_protocol(TinyOTShare::Protocol& protocol);

	void set_batch_size(int batch_size);
	void get_random_abit(emp::block& MAC, emp::block& KEY);

	TinyOTShare get_bit();

	void get_tinyot_triple(emp::block& aMAC, emp::block& aKEY,
		emp::block& bMAC, emp::block& bKEY,
		emp::block& cMAC, emp::block& cKEY);
	~BufferTinyOTPrep();
};

#endif 