#ifndef _SHA1
#define _SHA1

#include <sodium.h>

class octetStream;

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
	void final(unsigned char hashout[hash_length])
	{
		crypto_generichash_final(state, hashout, crypto_generichash_BYTES);
	}
	void final(octetStream& os);
	octetStream final();
};

void hash_update(Hash *ctx, const void *dataIn, unsigned long len);

#endif
