
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

atomic<int> BufferTinyOTPrep::port_resource(20000);

int BufferTinyOTPrep::get_next_available_port() {
	return ++port_resource;
}

BufferTinyOTPrep::BufferTinyOTPrep(DataPositions& usage, int port, int batch_size) :
	Preprocessing<TinyOTShare>(usage), io(nullptr), fpre(nullptr), P(nullptr),
	player_2pc(nullptr), port(port), batch_size(batch_size) {
	if (this->port < 0)
		this->port = get_next_available_port();
}

void BufferTinyOTPrep::set_protocol(TinyOTShare::Protocol& protocol) {
	this->P = &protocol.P;
	// [zico] Assuming 2pc, so the other player's number is (1 - my_num)
	player_2pc = new RealTwoPartyPlayerWithStats(*(this->P), 1 - this->P->my_num(), this->P->get_id() + "-TinyOT");
	// [zico] might need to update IP here according to MP SPDZ's configuration
	io = new EmpChannel(player_2pc);
	set_batch_size(batch_size);
}

void BufferTinyOTPrep::set_batch_size(int batch_size) {
	assert(io != nullptr);
	if (fpre)
		fpre->set_batch_size(batch_size);
	else 
		fpre = new emp::Fpre<EmpChannel>(io, this->P->my_num() + 1, batch_size);
		
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
	if (io)
		delete io;
	if (fpre)
		delete fpre;
	if (player_2pc)
		delete player_2pc;
}


#endif 