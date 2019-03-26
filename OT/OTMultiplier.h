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
};

template <class T, class U, class V, class W, class X>
class OTMultiplier : public OTMultiplierBase
{
    BitVector keyBits;
    vector< vector<BitVector> > senderOutput;
    vector<BitVector> receiverOutput;

    void multiplyForTriples();
    void multiplyForBits();

    virtual void after_correlation() = 0;

public:
    NPartyTripleGenerator& generator;
    int thread_num;
    OTExtensionWithMatrix rot_ext;
    OTCorrelator<Matrix<W> > auth_ot_ext;
    OTCorrelator<Matrix<X> > otCorrelator;
    vector< vector<V> > macs;

    OTMultiplier(NPartyTripleGenerator& generator, int thread_num);
    virtual ~OTMultiplier();
    void multiply();
};

template <class T>
class MascotMultiplier : public OTMultiplier<T, T, T, square128, square128>
{
    void after_correlation();

public:
    vector<T> c_output;

    MascotMultiplier(NPartyTripleGenerator& generator, int thread_num);
};

template <int K, int S>
class Spdz2kMultiplier: public OTMultiplier<Z2<K + S>, Z2<S>, Z2<K + S>,
        Z2kRectangle<K + S, K + S>, Z2kRectangle<TAU(K, S), K + S> >
{
    void after_correlation();

public:
    static const int TAU = TAU(K, S);
    static const int MAC_BITS = K + S;

    vector<Z2kRectangle<TAU, K + S> > c_output;

    Spdz2kMultiplier(NPartyTripleGenerator& generator, int thread_num) :
            OTMultiplier<Z2<MAC_BITS>, Z2<S>, Z2<MAC_BITS>,
                    Z2kRectangle<MAC_BITS, MAC_BITS>,
                    Z2kRectangle<TAU, MAC_BITS> >(generator, thread_num)
    {
    }
};

#endif /* OT_OTMULTIPLIER_H_ */
