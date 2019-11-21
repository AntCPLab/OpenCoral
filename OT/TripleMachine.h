/*
 * TripleMachine.h
 *
 */

#ifndef OT_TRIPLEMACHINE_H_
#define OT_TRIPLEMACHINE_H_

#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Z2k.h"
#include "Tools/OfflineMachineBase.h"
#include "OT/OTTripleSetup.h"

class GeneratorThread;

class MascotParams : virtual public OfflineParams
{
public:
    string prep_data_dir;
    bool generateMACs;
    bool amplify;
    bool check;
    bool correlation_check;
    bool generateBits;
    bool use_extension;
    bool fewer_rounds;
    bool fiat_shamir;
    struct timeval start, stop;

    MascotParams();

    void set_passive();
};

class TripleMachine : public OfflineMachineBase, public MascotParams
{
    Names N[2];
    int nConnections;

    gf2n mac_key2;
    gfp1 mac_keyp;
    Z2<128> mac_keyz;

public:
    int nloops;
    bool primeField;
    bool bonding;
    int z2k, z2s;

    TripleMachine(int argc, const char** argv);

    template<class T>
    GeneratorThread* new_generator(OTTripleSetup& setup, int i,
            typename T::mac_key_type mac_key);

    void run();

    void output_mac_keys();
};

#endif /* OT_TRIPLEMACHINE_H_ */
