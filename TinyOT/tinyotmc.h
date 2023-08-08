
#ifndef TINYOT_TINYOTMC_H__
#define TINYOT_TINYOTMC_H__

#include "TinyOT/tinyotshare.h"

class TinyOTMC: public MAC_Check_Base<TinyOTShare> {
public:
	TinyOTMC(TinyOTShare::mac_key_type key) {}

	void prepare_open(const TinyOTShare& secret, int = -1)
	{
		values.push_back(secret.get_share());
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
			this->values[i] ^= other.get_share();
		}
    }
};

#endif 