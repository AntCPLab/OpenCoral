#ifndef EMP_CHANNEL_H_
#define EMP_CHANNEL_H_

#include <emp-tool/emp-tool.h>
#include "Player.h"
#include "PlayerBuffer.h"

class EmpChannel: public emp::IOChannel<EmpChannel> {
	TwoPartyPlayer* P;
public:

	EmpChannel(TwoPartyPlayer* player): P(player) {
	}

	~EmpChannel() {
	}

	TwoPartyPlayer* get_player() {
		return P;
	}

	void flush() {}

	void send_data_internal(const void * data, size_t len) {
		// [zico] might need to revisit here because most places in MP SPDZ use the send API that accepts an octetStream.
		// But sending octetStream will waste some bandwidth by sending the length of data first.
		P->send(PlayerBuffer((octet*) data, len), true);
	}

	void recv_data_internal(void * data, size_t len) {
		P->recv(PlayerBuffer((octet*) data, len), true);
	}

	void send_bool(bool * data, size_t length) {
		uint8_t* packed_data = new uint8_t[(length + 7) / 8];
		memset(packed_data, 0, (length + 7) / 8);
		// pack bool
		for (size_t i = 0; i < length; i++) {
			packed_data[i / 8] ^= ((uint8_t)data[i]) << (i % 8);
		}
		send_data_internal(packed_data, (length+7)/8);
		delete[] packed_data;
	}

	void recv_bool(bool * data, size_t length) {
		uint8_t* packed_data = new uint8_t[(length + 7) / 8];
		memset(packed_data, 0, (length + 7) / 8);
		recv_data_internal(packed_data, (length+7)/8);
		// unpack bool
		for (size_t i = 0; i < length; i++) {
			data[i] = (packed_data[i / 8] >> (i % 8)) & 1;
		}
		delete[] packed_data;
	}

};

#endif