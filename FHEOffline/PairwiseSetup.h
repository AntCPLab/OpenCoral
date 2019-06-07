/*
 * PairwiseSetup.h
 *
 */

#ifndef FHEOFFLINE_PAIRWISESETUP_H_
#define FHEOFFLINE_PAIRWISESETUP_H_

#include "FHE/FHE_Params.h"
#include "FHE/Plaintext.h"
#include "Networking/Player.h"

class PairwiseMachine;

template <class FD>
class PairwiseSetup
{
    typedef typename FD::T T;

public:
    FHE_Params params;
    FD FieldD;
    typename FD::T alphai;
    Plaintext_<FD> alpha;
    string dirname;

    PairwiseSetup() : params(0), alpha(FieldD) {}
   
    void init(const Player& P, int sec, int plaintext_length, int& extra_slack);

    void secure_init(Player& P, PairwiseMachine& machine, int plaintext_length, int sec);
    void check(Player& P, PairwiseMachine& machine);
    void covert_key_generation(Player& P, PairwiseMachine& machine, int num_runs);
    void covert_mac_generation(Player& P, PairwiseMachine& machine, int num_runs);

    void set_alphai(T alphai);
};

#endif /* FHEOFFLINE_PAIRWISESETUP_H_ */
