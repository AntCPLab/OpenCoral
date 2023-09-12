#ifndef LEAKY_DELTA_OT_SILENT_H__
#define LEAKY_DELTA_OT_SILENT_H__
#include <emp-ot/emp-ot.h>
#include "GC/SilentOT.h"
namespace emp {
#ifdef __GNUC__
	#ifndef __clang__
		#pragma GCC push_options
		#pragma GCC optimize ("unroll-loops")
	#endif
#endif

template<typename T>
class LeakyDeltaOT {
public:
	T* ios[1];
	SilentOT<T>* ot;
	LeakyDeltaOT(int party, T * io) {
		ios[0] = io;
		ot = new SilentOT<T>(party, 1, ios, false, false, "", false);
	}

	~LeakyDeltaOT() {
		delete ot;
	}

	void setup_send(emp::block Delta, std::string pre_file) {
		ot->ferret->setup(Delta, pre_file);
	}

	void setup_recv(std::string pre_file) {
		ot->ferret->setup(pre_file);
	}

	emp::block Delta() {
		return ot->ferret->Delta;
	}
	
	void send_dot(block * data, int length) {
		ot->send_ot_rcm_rc(data, length);
		this->ios[0]->flush();
		block one = makeBlock(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE);
		for (int i = 0; i < length; ++i) {
			data[i] = data[i] & one;
		}
	}
	void recv_dot(block* data, int length) {
		bool * b = new bool[length];
		ot->recv_ot_rcm_rc(data, b, length);
		this->ios[0]->flush();

		block ch[2];
		ch[0] = zero_block;
		ch[1] = makeBlock(0, 1);
		block one = makeBlock(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE);
		for (int i = 0; i < length; ++i) {
			data[i] = (data[i] & one) ^ ch[b[i]];
		}
		delete[] b;
	}
};

#ifdef __GNUC_
	#ifndef __clang___
		#pragma GCC pop_options
	#endif
#endif
}
#endif// LEAKY_DELTA_OT_H__