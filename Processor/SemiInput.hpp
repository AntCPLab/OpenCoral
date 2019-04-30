/*
 * SemiInput.cpp
 *
 */

#include "SemiInput.h"

#include "ShamirInput.hpp"

template<class T>
void SemiInput<T>::add_mine(const typename T::clear& input)
{
	auto& P = this->P;
	typename T::open_type sum, share;
	for (int i = 0; i < P.num_players(); i++)
	{
		if (i < P.num_players() - 1)
			share.randomize(secure_prng);
		else
			share = input - sum;
		sum += share;
        if (i == P.my_num())
        	this->shares.push_back(share);
        else
        	share.pack(this->os[i]);
	}
}
