
#ifndef TINYOT_TINYOT_H__
#define TINYOT_TINYOT_H__

#include "TinyOT/fpre.h"
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"

class BlockType {
public:
	typedef emp::block Scalar;
	emp::block value;
};

class TinyOTShare {
public:
	emp::block MAC;
	emp::block KEY;

	typedef Bit open_type;
	typedef Bit clear;
	typedef BlockType mac_key_type;

	TinyOTShare() {}

	TinyOTShare(const emp::block& MAC, const emp::block& KEY) {
		this->MAC = MAC;
		this->KEY = KEY;
	}

	Bit get_bit() const {
		return emp::getLSB(MAC);
	}

	void pack(octetStream& o) const {
		o.append((octet*) &MAC, sizeof(emp::block));
	}

	void unpack(octetStream& o) { 
		o.consume((octet*) &MAC, sizeof(emp::block)); 
	}

	void mul(const TinyOTShare& S,const bool& c) {
		if (c) {
			*this = S;
		}
		else {
			this->MAC = emp::zero_block;
			this->KEY = emp::zero_block;
		}
	}
   	void add(const TinyOTShare& S1, const TinyOTShare& S2) {
		this->MAC = S1.MAC ^ S2.MAC;
		this->KEY = S1.KEY ^ S2.KEY;
	}

	TinyOTShare operator+(const TinyOTShare& other) const {
		TinyOTShare res;
		res.add(*this, other);
		return res;
	}

	TinyOTShare operator*(bool c) const {
		TinyOTShare res;
		res.mul(*this, c);
		return res;
	}

	TinyOTShare& operator+=(const TinyOTShare& x) { 
		add(*this, x); return *this;
	}

};

class BufferTinyOTPrep {
	emp::NetIO *io;
	emp::Fpre<emp::NetIO>* fpre;
	vector<emp::block> random_abit_MACs;
	vector<emp::block> random_abit_KEYs;

public:

	BufferTinyOTPrep(int party, int port) {
		io = new emp::NetIO(party==emp::ALICE ? nullptr:emp::IP, port);
		fpre = new emp::Fpre<emp::NetIO>(io, party, 1000);

		random_abit_MACs.reserve(fpre->batch_size);
		random_abit_KEYs.reserve(fpre->batch_size);
	}

	void get_random_abit(emp::block& MAC, emp::block& KEY) {
		if(random_abit_MACs.empty()) {
			random_abit_MACs.resize(fpre->batch_size);
			random_abit_KEYs.resize(fpre->batch_size);
			fpre->random_abit(random_abit_MACs.data(), random_abit_KEYs.data());
		}
		MAC = random_abit_MACs.back();
		KEY = random_abit_KEYs.back();
		random_abit_MACs.pop_back();
		random_abit_KEYs.pop_back();
	}

	~BufferTinyOTPrep() {
		delete io;
		delete fpre;
	}

};

class TinyOTMC: public MAC_Check_Base<TinyOTShare> {
public:
	void prepare_open(const TinyOTShare& secret, int = -1)
	{
		values.push_back(secret.get_bit());
		secrets.push_back(secret);
	}

	// [TODO] need to update for security and performanace (batch check)
	void exchange(const Player& P) {
        vector<octetStream> oss;
        oss.resize(P.num_players());
        oss[P.my_num()].reset_write_head();
		// [TODO] this needs to be updated accordingly with TinyOTShare::pack
        oss[P.my_num()].reserve(this->secrets.size() * sizeof(emp::block)); 

        for (auto& x : this->secrets)
            x.pack(oss[P.my_num()]);
        oss[P.my_num()].append(0);

        P.unchecked_broadcast(oss);

		for (size_t i = 0; i < this->values.size(); i++) {
			TinyOTShare other;
			other.unpack(oss[1 - P.my_num()]);
			this->values[i] ^= other.get_bit();
		}
    }
};

// bool mock_tinyot_open(Player* P, const TinyOTShare& share) {
//     octetStream os;
//     share.pack(os);
//     P->exchange_relative(-1, os);
//     TinyOTShare other_share;
//     other_share.unpack(os);

//     TinyOTShare revealed = (share + other_share);

//     return revealed.get_bit();
// }

#endif 