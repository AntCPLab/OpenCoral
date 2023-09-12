
#ifndef TINYOT_TINYOTSHARE_H__
#define TINYOT_TINYOTSHARE_H__

#include <emp-tool/emp-tool.h>
#include "Tools/octetStream.h"
#include "Protocols/MAC_Check_Base.h"
#include "Math/Bit.h"
#include "Tools/debug.h"
#include "Protocols/fake-stuff.h"
#include "Protocols/ShareInterface.h"


class BlockType {
public:
	typedef emp::block Scalar;
	emp::block value;

	void randomize(PRNG& G)
	{
		G.get_octets<16>((octet*)&value);
	}

	static string type_short() {
		return "BLK";
	}

	void input(istream& s, bool = true) {
		s.read((char*) &value, 16);
	}

	static void init_field() {}
};

class TinyOTMC;
class BufferTinyOTPrep;
class TinyOTProtocol;
class TinyOTInput;

class TinyOTShare: public ShareInterface {
public:
	emp::block MAC;
	emp::block KEY;

	typedef TinyOTShare This;
	typedef Bit open_type;
	typedef Bit clear;
	typedef BlockType mac_key_type;
	typedef TinyOTMC MAC_Check;
	typedef MAC_Check MC;
	typedef BufferTinyOTPrep LivePrep;
	typedef TinyOTProtocol Protocol;
	typedef TinyOTInput Input;
	typedef This bit_type;
	typedef This part_type;
	typedef This small_type;

	static const int default_length = 1;
	const static bool randoms_for_opens = false;
	const static bool is_real = true;

	TinyOTShare() {}

	TinyOTShare(const emp::block& MAC, const emp::block& KEY) {
		this->MAC = MAC;
		this->KEY = KEY;
	}

	/**
	 * This api exists to be compabible with the vectorized SS type.
	*/
	const This& get_bit(int i) const {
		assert(i == 0);
		return *this;
	}

	Bit get_share() const {
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

	/**
	 * Just for compatibility. Generated key never used in real protocol.
	*/
	static void read_or_generate_mac_key(string directory, const Player& P, mac_key_type& key) {
		try
		{
			read_mac_key(directory, P.N, key);
		}
		catch (mac_key_error&)
		{
			SeededPRNG G;
			key.randomize(G);
		}
	}

	static MC* new_mc(mac_key_type key);

};

#endif 