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

};

#endif