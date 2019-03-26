// (C) 2018 University of Bristol. See License.txt

/*
 * OTMultiplier.h
 *
 */

#ifndef OT_OTMULTIPLIER_H_
#define OT_OTMULTIPLIER_H_

#include <vector>
using namespace std;

#include "OT/OTExtensionWithMatrix.h"
#include "OT/OTVole.h"
#include "OT/Rectangle.h"
#include "Tools/random.h"

class NPartyTripleGenerator;

class OTMultiplierBase
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t ready;
    pthread_t thread;

    virtual ~OTMultiplierBase() {}
    virtual void multiply() = 0;

    void signal_generator() { pthread_cond_signal(&ready); }
    void wait_for_generator() { pthread_cond_wait(&ready, &mutex); }
};

template <class V>
class OTMultiplierMac : public OTMultiplierBase
{
public:
    vector< vector<V> > macs;
};

template <class T, class U, class V, class X>
class OTMultiplier : public OTMultiplierMac<V>
{
protected:
    BitVector keyBits;
    vector< vector<BitVector> > senderOutput;
    vector<BitVector> receiverOutput;

    void multiplyForTriples();
    void multiplyForBits();

    virtual void after_correlation() = 0;
    virtual void init_authenticator(const BitVector& baseReceiverInput,
            const vector< vector<BitVector> >& baseSenderInput,
            const vector<BitVector>& baseReceiverOutput) = 0;

public:
    NPartyTripleGenerator& generator;
    int thread_num;
    OTExtensionWithMatrix rot_ext;
    OTCorrelator<Matrix<X> > otCorrelator;

    OTMultiplier(NPartyTripleGenerator& generator, int thread_num);
    virtual ~OTMultiplier();
    void multiply();
};

template <class T>
class MascotMultiplier : public OTMultiplier<T, T, T, square128>
{
    OTCorrelator<Matrix<square128> > auth_ot_ext;
    void after_correlation();
    void init_authenticator(const BitVector& baseReceiverInput,
            const vector< vector<BitVector> >& baseSenderInput,
            const vector<BitVector>& baseReceiverOutput);

public:
    vector<T> c_output;

    MascotMultiplier(NPartyTripleGenerator& generator, int thread_num);
};

// values, key, mac, mult-rectangle
template <int K, int S>
class Spdz2kMultiplier: public OTMultiplier<Z2<K + S>, Z2<S>, Z2<K + 2 * S>,
	Z2kRectangle<TAU(K, S), K + S> >
{
    void after_correlation();
    void init_authenticator(const BitVector& baseReceiverInput,
            const vector< vector<BitVector> >& baseSenderInput,
            const vector<BitVector>& baseReceiverOutput);

public:
    static const int TAU = TAU(K, S);
    static const int PASSIVE_MULT_BITS = K + S;
    static const int MAC_BITS = K + 2 * S;

    vector<Z2kRectangle<TAU, PASSIVE_MULT_BITS> > c_output;
    OTVoleBase<Z2<MAC_BITS>, Z2<S>>* mac_vole;

    Spdz2kMultiplier(NPartyTripleGenerator& generator, int thread_num);
};

#endif /* OT_OTMULTIPLIER_H_ */
