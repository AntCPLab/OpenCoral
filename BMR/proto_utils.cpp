// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * utils.cpp
 *
 */



#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
//#include <intrin.h>
#include <stdlib.h>

#include "proto_utils.h"

void fill_message_type(void* buffer, MSG_TYPE type)
{
	memcpy(buffer, &type, sizeof(MSG_TYPE));
}

void fill_message_type(SendBuffer& buffer, MSG_TYPE type)
{
	buffer.serialize(type);
}



//inline void xor_big(const char* input1, const char* input2, char* output)
//{

//	static void XOR(block in1[], block in2[], block out[], int sliceLen){
//
//	#ifdef NO_VEC_OPT
//	for (int i=0; i<sliceLen; i++){
//	    out[i] = in1[i] ^ in2[i];
//	 }
//	#else
//
//	    const unsigned int u = 0xFFFFFFFF;
//	    __m256 in1_vec, in2_vec, out_vec;
//	    __m256i ALL_ONES_256 = _mm256_set_epi32(u,u,u,u,u,u,u,u);
//	    __m256i *output_as_int_vec;
//
//	    float *fin1 = (float *)in1;
//	    float *fin2 = (float *)in2;
//	    float *fout = (float *)out;
//
//
//	    for (int i=0; i<sliceLen; i+=8){
//			in1_vec = _mm256_load_ps(fin1 + i);
//				in2_vec = _mm256_load_ps(fin2 + i);
//				out_vec = _mm256_xor_ps(in1_vec, in2_vec);
//				output_as_int_vec = (__m256i* ) &out_vec;
//			_mm256_maskstore_epi32((int *)fout,ALL_ONES_256,(*output_as_int_vec));
//
//	    }
//
//	#endif
//	}
//}
