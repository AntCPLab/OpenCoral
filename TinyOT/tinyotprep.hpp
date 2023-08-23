
#ifndef TINYOT_TINYOTPREP_HPP__
#define TINYOT_TINYOTPREP_HPP__

#include "TinyOT/fpre.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "TinyOT/tinyotshare.h"
#include "tinyotprep.h"
#include "tinyotprotocol.h"

BufferTinyOTPrep::BufferTinyOTPrep(DataPositions& usage, int port, int batch_size) :
	Preprocessing<TinyOTShare>(usage), io(nullptr), fpre(nullptr), P(nullptr),
	port(port), batch_size(batch_size) {
}

void BufferTinyOTPrep::set_protocol(TinyOTShare::Protocol& protocol) {
	this->P = &protocol.P;
	io = new emp::NetIO(this->P->my_num() + 1 == emp::ALICE ? nullptr:emp::IP, port);
	set_batch_size(batch_size);
}

void BufferTinyOTPrep::set_batch_size(int batch_size) {
	assert(io != nullptr);
	if (fpre)
		delete fpre;
	fpre = new emp::Fpre<emp::NetIO>(io, this->P->my_num() + 1, batch_size);
	triple_buf_idx = batch_size;

	random_abit_MACs.reserve(fpre->batch_size);
	random_abit_KEYs.reserve(fpre->batch_size);
}

void BufferTinyOTPrep::get_random_abit(emp::block& MAC, emp::block& KEY) {
	if(random_abit_MACs.empty()) {
		print_general("refill random abit buffer: ", fpre->batch_size);
		random_abit_MACs.resize(fpre->batch_size);
		random_abit_KEYs.resize(fpre->batch_size);
		fpre->random_abit(random_abit_MACs.data(), random_abit_KEYs.data());
	}
	MAC = random_abit_MACs.back();
	KEY = random_abit_KEYs.back();
	random_abit_MACs.pop_back();
	random_abit_KEYs.pop_back();
}

TinyOTShare BufferTinyOTPrep::get_bit() {
	TinyOTShare res;
	get_random_abit(res.MAC, res.KEY);
	return res;
}

void BufferTinyOTPrep::get_tinyot_triple(
	emp::block& aMAC, emp::block& aKEY,
	emp::block& bMAC, emp::block& bKEY,
	emp::block& cMAC, emp::block& cKEY) {
	if (triple_buf_idx >= fpre->batch_size) {
		print_general("refill triple buffer: ", fpre->batch_size);
		fpre->refill();
		triple_buf_idx = 0;
	}
	aMAC = fpre->MAC_res[triple_buf_idx*3];
	aKEY = fpre->KEY_res[triple_buf_idx*3];
	bMAC = fpre->MAC_res[triple_buf_idx*3 + 1];
	bKEY = fpre->KEY_res[triple_buf_idx*3 + 1];
	cMAC = fpre->MAC_res[triple_buf_idx*3 + 2];
	cKEY = fpre->KEY_res[triple_buf_idx*3 + 2];
	triple_buf_idx++;
}

BufferTinyOTPrep::~BufferTinyOTPrep() {
	delete io;
	delete fpre;
}


#endif 