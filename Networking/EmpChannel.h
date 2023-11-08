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

	void send_bool_aligned(const bool * data, size_t length) {
		const bool * data64 = data;
		size_t i = 0;
		unsigned long long unpack;
		uint8_t* packed_data = new uint8_t[length/8];
		for(; i < length/8; ++i) {
			unsigned long long mask = 0x0101010101010101ULL;
			unsigned long long tmp = 0;
			memcpy(&unpack, data64, sizeof(unpack));
			data64 += sizeof(unpack);
#if defined(__BMI2__)
			tmp = _pext_u64(unpack, mask);
			// cout << "[zico] use bmi2 pext" << endl;
#else
			// https://github.com/Forceflow/libmorton/issues/6
			for (unsigned long long bb = 1; mask != 0; bb += bb) {
				if (unpack & mask & -mask) { tmp |= bb; }
				mask &= (mask - 1);
			}
#endif
			packed_data[i] = (uint8_t) tmp;
		}
		send_data(packed_data, length/8);
		if (8*i != length)
			send_data(data + 8*i, length - 8*i);
		
		delete [] packed_data;
	}

	void recv_bool_aligned(bool * data, size_t length) {
		bool * data64 = data;
		size_t i = 0;
		unsigned long long unpack;
		uint8_t* packed_data = new uint8_t[length/8];
		recv_data(packed_data, length/8);
		for(; i < length/8; ++i) {
			unsigned long long mask = 0x0101010101010101ULL;
			unsigned long long tmp = packed_data[i];
#if defined(__BMI2__)
			unpack = _pdep_u64(tmp, mask);
#else
			unpack = 0;
			for (unsigned long long bb = 1; mask != 0; bb += bb) {
				if (tmp & bb) {unpack |= mask & (-mask); }
				mask &= (mask - 1);
			}
#endif
			memcpy(data64, &unpack, sizeof(unpack));
			data64 += sizeof(unpack);
		}
		if (8*i != length)
			recv_data(data + 8*i, length - 8*i);

		delete [] packed_data;
	}

	void send_bool(bool * data, size_t length) {
		void * ptr = (void *)data;
		size_t space = length;
		const void * aligned = std::align(alignof(uint64_t), sizeof(uint64_t), ptr, space);
		if(aligned == nullptr)
			send_data(data, length);
		else{
			size_t diff = length - space;
			send_data(data, diff);
			send_bool_aligned((const bool*)aligned, length - diff);
		}
	}

	void recv_bool(bool * data, size_t length) {
		void * ptr = (void *)data;
		size_t space = length;
		void * aligned = std::align(alignof(uint64_t), sizeof(uint64_t), ptr, space);
		if(aligned == nullptr)
			recv_data(data, length);
		else{
			size_t diff = length - space;
			recv_data(data, diff);
			recv_bool_aligned((bool*)aligned, length - diff);
		}
	}


	/**
	 * This function doesn't work probably due to compiler optimization.
	 * The conversion from bool to int is wrong.
	*/
	// void send_bool(bool * data, size_t length) {
	// 	uint8_t* packed_data = new uint8_t[(length + 7) / 8];
	// 	memset(packed_data, 0, (length + 7) / 8);
	// 	// pack bool
	// 	for (size_t i = 0; i < length; i++) {
	// 		// [zico] Converting bool to int type leads to very strange bug. It does NOT always give 0 or 1.
	// 		// Moreover, the compiler optimization eliminates a lot of operations, making the code not having any effect.
	// 		packed_data[i / 8] ^= (data[i]? 1 : 0) << (i % 8); // Doesn't work
	// 	}
	// 	send_data_internal(packed_data, (length+7)/8);
	// 	cout << "[zico] send data: ";
	// 	for (size_t i = 0; i < length && i < 80; i++) {
	// 		cout << (int)data[i] << ",";
	// 	}
	// 	cout << endl;
		
	// 	cout << hex << (int) packed_data[0] << dec << endl;

	// 	delete[] packed_data;
	// }

	// void recv_bool(bool * data, size_t length) {
	// 	uint8_t* packed_data = new uint8_t[(length + 7) / 8];
	// 	memset(packed_data, 0, (length + 7) / 8);
	// 	recv_data_internal(packed_data, (length+7)/8);
	// 	// unpack bool
	// 	for (size_t i = 0; i < length; i++) {
	// 		data[i] = (packed_data[i / 8] >> (i % 8)) & 1;
	// 	}
	// 	cout << "[zico] recv data: ";
	// 	for (size_t i = 0; i < length && i < 80; i++)
	// 		cout << (int)data[i] << ",";
	// 	cout << endl;
	// 	delete[] packed_data;
	// }

};

#endif