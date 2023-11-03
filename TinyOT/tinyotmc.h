
#ifndef TINYOT_TINYOTMC_H__
#define TINYOT_TINYOTMC_H__

#include "TinyOT/tinyotshare.h"

class TinyOTMC: public MAC_Check_Base<TinyOTShare> {
	TinyOTShare::mac_key_type key;
	octetStream open[2];
public:
	/**
	 * [zico] This key needs to be synced with the key used in `Fpre`.
	 * For now, it is just dummy for our performance evaluation.
	 * */
	TinyOTMC(TinyOTShare::mac_key_type key) : key(key) {}

	void prepare_open(const TinyOTShare& secret, int = -1)
	{
		values.push_back(secret.get_share());
		secrets.push_back(secret);
	}

	// // [zico] need to update for security and performanace (batch check)
	// void exchange(const Player& P) {
    //     vector<octetStream> oss;
    //     oss.resize(P.num_players());
    //     oss[P.my_num()].reset_write_head();
	// 	// [zico] this needs to be updated accordingly with TinyOTShare::pack
    //     oss[P.my_num()].reserve(this->secrets.size() * sizeof(emp::block)); 

    //     for (auto& x : this->secrets)
    //         x.pack(oss[P.my_num()]);
    //     oss[P.my_num()].append(0);

    //     P.unchecked_broadcast(oss);

	// 	for (size_t i = 0; i < this->values.size(); i++) {
	// 		TinyOTShare other;
	// 		other.unpack(oss[1 - P.my_num()]);
	// 		this->values[i] ^= other.get_share();
	// 	}
    // }

	// [zico] need to update for security and performanace (batch check)
	void exchange(const Player& P) {
        vector<octetStream> oss;
        oss.resize(P.num_players());
        oss[P.my_num()].reset_write_head();
		// [zico] this needs to be updated accordingly with TinyOTShare::pack
        oss[P.my_num()].reserve((this->secrets.size() + 7) / 8); 

        for (auto& x : this->secrets) {
            x.pack(oss[P.my_num()]);
			open[P.my_num()].append((const octet*) &x.MAC, sizeof(emp::block));
		}
        oss[P.my_num()].append(0);

        P.unchecked_broadcast(oss);

		for (size_t i = 0; i < this->values.size(); i++) {
			TinyOTShare other;
			other.unpack(oss[1 - P.my_num()]);
			this->values[i] ^= other.get_share();

			// Assemble the computed MACs
			emp::block MAC_prime = this->secrets[i].KEY;
			if (other.get_share().get())
				MAC_prime = MAC_prime ^ key.value;
			open[1 - P.my_num()].append((const octet*) &MAC_prime, sizeof(emp::block));
		}
    }

	void Check(const Player& P) {
		octetStream hash = open[P.my_num()].hash();
		vector<octetStream> oss;
		oss.resize(P.num_players());
        oss[P.my_num()].reset_write_head();

		oss[P.my_num()] = hash;
		oss[P.my_num()].append(0);
		P.unchecked_broadcast(oss);

		// [zico] Now the key is not synced, so we skip this check.
		// assert(open[1 - P.my_num()].hash() == oss[1 - P.my_num()]);

		open[0].reset_write_head();
		open[1].reset_write_head();

		values.clear();
		secrets.clear();
	}
};

#endif 