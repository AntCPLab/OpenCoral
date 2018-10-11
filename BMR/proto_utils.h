/*
 * utils.h
 *
 */

#ifndef PROTO_UTILS_H_
#define PROTO_UTILS_H_

#include "msg_types.h"
#include <time.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
using namespace std;

#include "Tools/avx_memcpy.h"
#include "Tools/FlexBuffer.h"

#define LOOPBACK_STR "LOOPBACK"

class SendBuffer;

void fill_message_type(void* buffer, MSG_TYPE type);
void fill_message_type(SendBuffer& buffer, MSG_TYPE type);

void phex (const void *addr, int len);

//inline void xor_big(const char* input1, const char* input2, char* output);


inline timeval GET_TIME() {
	struct timeval now;
	int rc = gettimeofday(&now, 0);
    if (rc != 0) {
        perror("gettimeofday");
    }
	return now;
}

inline unsigned long GET_DIFF(struct timeval* before, struct timeval* after) {
	long diff = ((after->tv_sec - before->tv_sec)*1000000L
	           +after->tv_usec) - before->tv_usec;
	return diff;
}

inline unsigned long PRINT_DIFF(struct timeval* before, struct timeval* after) {
	long diff = GET_DIFF(before, after);
	printf("Time in microseconds: %ld us\n", diff );
	return diff;
}

inline void phex(const FlexBuffer& buffer) { phex(buffer.data(), buffer.size()); }

inline void print_bit_array(const char* bits, int len)
{
	for (int i = 0; i < len; i++)
	{
		if (i % 8 == 0)
			cout << " ";
		cout << (int)bits[i];
	}
	cout << endl;
}

inline void print_bit_array(const vector<char>& bits)
{
	print_bit_array(bits.data(), bits.size());
}

inline void print_indices(const vector<int>& indices)
{
	for (unsigned i = 0; i < indices.size(); i++)
		cout << indices[i] << " ";
	cout << endl;
}

#endif /* NETWORK_TEST_UTILS_H_ */
