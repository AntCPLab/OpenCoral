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
class MachineBase;

template <class T>
void secure_init(T& setup, Player& P, MachineBase& machine,
        int plaintext_length, int sec);

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

    static string name()
    {
        return "PairwiseParams-" + FD::T::type_string();
    }

    PairwiseSetup() : params(0), alpha(FieldD) {}
   
    void init(const Player& P, int sec, int plaintext_length, int& extra_slack);

    void secure_init(Player& P, PairwiseMachine& machine, int plaintext_length, int sec);
    void generate(Player& P, MachineBase& machine, int plaintext_length, int sec);
    void check(Player& P, MachineBase& machine);
    void covert_key_generation(Player& P, PairwiseMachine& machine, int num_runs);
    void covert_mac_generation(Player& P, PairwiseMachine& machine, int num_runs);

    void pack(octetStream& os) const;
    void unpack(octetStream& os);

    void set_alphai(T alphai);
};

#endif /* FHEOFFLINE_PAIRWISESETUP_H_ */
