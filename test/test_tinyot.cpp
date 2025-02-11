#define VERBOSE_DEBUG_PRINT
#include "TinyOT/fpre_old.h"

void test_refill(int party, int port) {
    int N = 10000;

	emp::NetIO *io;
	io = new emp::NetIO(party==emp::ALICE ? nullptr:emp::IP, port);
	emp::Fpre<emp::NetIO> * fpre = new emp::Fpre<emp::NetIO>(io, party, N);
	auto tt1 = emp::clock_start();
	fpre->refill();
	cout << emp::time_from(tt1)/(N)*1000<<endl;
	cout << fpre->bandwidth() / 1000 << " KB" <<endl;	

	fpre->check_correctness(fpre->MAC_res, fpre->KEY_res, fpre->batch_size);	
	delete fpre;
	delete io;
}

int main(int argc, char** argv) {
    int port, party;
	emp::parse_party_and_port(argv, &party, &port);

    test_refill(party, port);

    return 0;
}