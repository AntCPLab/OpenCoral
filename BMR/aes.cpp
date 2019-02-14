#include "aes.h"
#include <stdexcept>

#ifdef _WIN32
#include "StdAfx.h"
#endif

void AES_128_Key_Expansion(const unsigned char *userkey, AES_KEY *aesKey)
{
    block x0,x1,x2;
    //block *kp = (block *)&aesKey;
	aesKey->rd_key[0] = x0 = _mm_loadu_si128((block*)userkey);
    x2 = _mm_setzero_si128();
#ifdef __AES__
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);   aesKey->rd_key[1] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);   aesKey->rd_key[2] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);   aesKey->rd_key[3] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);   aesKey->rd_key[4] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);  aesKey->rd_key[5] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);  aesKey->rd_key[6] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);  aesKey->rd_key[7] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 128); aesKey->rd_key[8] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);  aesKey->rd_key[9] = x0;
	EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);  aesKey->rd_key[10] = x0;
#else
	(void) x1, (void) x2;
	throw std::runtime_error("need to compile with AES-NI support");
#endif
}
