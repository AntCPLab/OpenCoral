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

template<class T> class NPartyTripleGenerator;

class MascotParams : virtual public OfflineParams
{
protected:
    gf2n_short mac_key2s;
    gf2n_long mac_key2l;
    gfp1 mac_keyp;
    Z2<128> mac_keyz;

public:
    string prep_data_dir;
    bool generateMACs;
    bool amplify;
    bool check;
    bool generateBits;
    struct timeval start, stop;

    MascotParams();

    void set_passive();

    template <class T>
    T get_mac_key();
    template <class T>
    void set_mac_key(T key);
};

class TripleMachine : public OfflineMachineBase, public MascotParams
{
    Names N[2];
    int nConnections;

public:
    int nloops;
    bool primeField;
    bool bonding;
    int z2k, z2s;

    TripleMachine(int argc, const char** argv);

    template<class T>
    NPartyTripleGenerator<T>* new_generator(OTTripleSetup& setup, int i);

    void run();

    void output_mac_keys();
};

#endif /* OT_TRIPLEMACHINE_H_ */
