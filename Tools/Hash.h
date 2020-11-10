#ifndef _SHA1
#define _SHA1

#include <sodium.h>
#include <vector>
using namespace std;

#include "octetStream.h"

class Hash
{
	crypto_generichash_state* state;

public:
	static const int hash_length = crypto_generichash_BYTES;

	unsigned size;

	Hash();
	~Hash();

	void reset();

	void update(const void *dataIn, unsigned long len)
	{
		crypto_generichash_update(state, (unsigned char*)dataIn, len);
		size += len;
	}
	void update(const octetStream& os);
	template<class T>
	void update(const T& x)
	{
	    update(x.get_ptr(), x.size());
	}
	template<class T>
	void update(const vector<T>& v)
	{
	    octetStream tmp(v.size() * sizeof(T));
	    for (auto& x : v)
	        x.pack(tmp);
	    update(tmp);
	}

	void final(unsigned char hashout[hash_length])
	{
		final(hashout, hash_length);
	}
	void final(unsigned char* hashout, size_t length)
	{
		crypto_generichash_final(state, hashout, length);
	}
	void final(octetStream& os);
	octetStream final();
};

void hash_update(Hash *ctx, const void *dataIn, unsigned long len);

#endif
