

/**
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
* Copyright(c) 2013 Ted Krovetz.
* This file is part of the SCAPI project, is was taken from the file ocb.c written by Ted Krovetz.
* Some changes and additions may have been made and only part of the file written by Ted Krovetz has been copied
* only for the use of this project.
*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*/

// Copyright(c) 2013 Ted Krovetz.

#ifndef TED_FILE
#define TED_FILE

#include <wmmintrin.h>
#include "defs.h"




typedef struct { block rd_key[15]; int rounds; } AES_KEY;

#define EXPAND_ASSIST(v1,v2,v3,v4,shuff_const,aes_const)                    \
    v2 = _mm_aeskeygenassist_si128(v4,aes_const);                           \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 16));        \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 140));       \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v2 = _mm_shuffle_epi32(v2,shuff_const);                                 \
    v1 = _mm_xor_si128(v1,v2)

void AES_128_Key_Expansion(const unsigned char *userkey, AES_KEY* aesKey);

#endif /* PROTOCOL_INC_AES_H_ */
