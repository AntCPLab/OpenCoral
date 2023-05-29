#include "TinyOT/fpre.h"

void test_refill(int party, int port) {
    int N = 1<<20;

	NetIO *io;
	io = new NetIO(party==ALICE ? nullptr:IP, port);
	Fpre<NetIO> * fpre = new Fpre<NetIO>(io, party, N);
	auto tt1 = clock_start();
	fpre->refill();
	cout << time_from(tt1)/(N)*1000<<endl;
	cout << fpre->bandwidth()<<endl;	

	fpre->check_correctness(fpre->MAC_res, fpre->KEY_res, fpre->batch_size);	
	delete fpre;
	delete io;
}

int main(int argc, char** argv) {
    int port, party;
	parse_party_and_port(argv, &party, &port);

    test_refill(party, port);

    return 0;
}