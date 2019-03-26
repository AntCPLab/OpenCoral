// (C) 2018 University of Bristol. See License.txt

/*
 * OTMultiplier.cpp
 *
 */

#include "OT/OTMultiplier.h"
#include "OT/NPartyTripleGenerator.h"
#include "OT/Rectangle.h"
#include "Math/Z2k.h"

#include <math.h>

//#define OTCORR_TIMER

template<class T, class U, class V, class X>
OTMultiplier<T, U, V, X>::OTMultiplier(NPartyTripleGenerator& generator,
        int thread_num) :
        generator(generator), thread_num(thread_num),
        rot_ext(128, 128, 0, 1,
                generator.players[thread_num], generator.baseReceiverInput,
                generator.baseSenderInputs[thread_num],
                generator.baseReceiverOutputs[thread_num], BOTH, !generator.machine.check),
        otCorrelator(0, 0, 0, 0, generator.players[thread_num], {}, {}, {}, BOTH, true)
{
    pthread_mutex_init(&this->mutex, 0);
    pthread_cond_init(&this->ready, 0);
    this->thread = 0;
}

template<class T>
MascotMultiplier<T>::MascotMultiplier(NPartyTripleGenerator& generator,
        int thread_num) :
        OTMultiplier<T, T, T, square128>(generator, thread_num),
		auth_ot_ext(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, true)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template <int K, int S>
Spdz2kMultiplier<K, S>::Spdz2kMultiplier(NPartyTripleGenerator& generator, int thread_num) :
        OTMultiplier<Z2<PASSIVE_MULT_BITS>, // bit length used when computing shares 
                Z2<S>, // bit length of key share
                Z2<MAC_BITS>, // bit length used when computing mac shares
                Z2kRectangle<TAU, PASSIVE_MULT_BITS> > // mult-rectangle
        (generator, thread_num)
{
#ifdef USE_OPT_VOLE
		mac_vole = new OTVole<Z2<MAC_BITS>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
#else
		mac_vole = new OTVoleBase<Z2<MAC_BITS>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
#endif
}

template<class T, class U, class V, class X>
OTMultiplier<T, U, V, X>::~OTMultiplier()
{
    pthread_mutex_destroy(&this->mutex);
    pthread_cond_destroy(&this->ready);
}

template<class T, class U, class V, class X>
void OTMultiplier<T, U, V, X>::multiply()
{
    keyBits.set(generator.machine.get_mac_key<U>());
    rot_ext.extend<gf2n>(keyBits.size(), keyBits);
    senderOutput.resize(keyBits.size());
    for (size_t j = 0; j < keyBits.size(); j++)
    {
        senderOutput[j].resize(2);
        for (int i = 0; i < 2; i++)
        {
            senderOutput[j][i].resize(128);
            senderOutput[j][i].set_int128(0, rot_ext.senderOutputMatrices[i].squares[0].rows[j]);
        }
    }
    rot_ext.receiverOutputMatrix.to(receiverOutput);
    receiverOutput.resize(keyBits.size());
    init_authenticator(keyBits, senderOutput, receiverOutput);

    if (generator.machine.generateBits)
        multiplyForBits();
    else
        multiplyForTriples();
}

template<class T, class U, class V, class X>
void OTMultiplier<T, U, V, X>::multiplyForTriples()
{
    // dummy input for OT correlator
    vector<BitVector> _;
    vector< vector<BitVector> > __;
    BitVector ___;

    otCorrelator.resize(X::N_COLUMNS * generator.nPreampTriplesPerLoop);

    rot_ext.resize(X::N_ROWS * generator.nPreampTriplesPerLoop + 2 * 128);
    
    vector<Matrix<X> >& baseSenderOutputs = otCorrelator.matrices;
    Matrix<X>& baseReceiverOutput = otCorrelator.senderOutputMatrices[0];

    pthread_mutex_lock(&this->mutex);
    this->signal_generator();
    this->wait_for_generator();

    for (int i = 0; i < generator.nloops; i++)
    {
        BitVector aBits = generator.valueBits[0];
        //timers["Extension"].start();
        rot_ext.extend_correlated(X::N_ROWS * generator.nPreampTriplesPerLoop, aBits);
        rot_ext.hash_outputs<T>(aBits.size(), baseSenderOutputs, baseReceiverOutput);
        //timers["Extension"].stop();

        //timers["Correlation"].start();
        otCorrelator.setup_for_correlation(aBits, baseSenderOutputs,
                baseReceiverOutput);
        otCorrelator.template correlate<T>(0, generator.nPreampTriplesPerLoop,
                generator.valueBits[1], false, generator.nAmplify);
        //timers["Correlation"].stop();

        //timers["Triple computation"].start();

        this->after_correlation();

        this->signal_generator();
        this->wait_for_generator();
    }

    pthread_mutex_unlock(&this->mutex);
}

template <class T>
void MascotMultiplier<T>::init_authenticator(const BitVector& keyBits,
		const vector< vector<BitVector> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->auth_ot_ext.init(keyBits, senderOutput, receiverOutput);
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::init_authenticator(const BitVector& keyBits,
		const vector< vector<BitVector> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
	this->mac_vole->init(keyBits, senderOutput, receiverOutput);
}

template <class T>
void MascotMultiplier<T>::after_correlation()
{
	this->auth_ot_ext.resize(this->generator.nPreampTriplesPerLoop * square128::N_COLUMNS);

    this->otCorrelator.reduce_squares(this->generator.nPreampTriplesPerLoop,
            this->c_output);

    if (this->generator.machine.generateMACs)
    {
        pthread_cond_signal(&this->ready);
        pthread_cond_wait(&this->ready, &this->mutex);

        this->macs.resize(3);
        for (int j = 0; j < 3; j++)
        {
            int nValues = this->generator.nTriplesPerLoop;
            if (this->generator.machine.check && (j % 2 == 0))
                nValues *= 2;
            this->auth_ot_ext.template expand<T>(0, nValues);
            this->auth_ot_ext.template correlate<T>(0, nValues,
                    this->generator.valueBits[j], true);
            this->auth_ot_ext.reduce_squares(nValues, this->macs[j]);
        }
    }
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nTriplesPerLoop,
            this->c_output);

    this->signal_generator();
    this->wait_for_generator();

    this->macs.resize(3);
#ifdef OTCORR_TIMER
        timeval totalstartv, totalendv;
        gettimeofday(&totalstartv, NULL);
#endif
    for (int j = 0; j < 3; j++)
    {
        int nValues = this->generator.nTriplesPerLoop;
        BitVector* bits; 
        if (//this->generator.machine.check &&
                (j % 2 == 0)){
            nValues *= 2;
            bits = &(this->generator.valueBits[j]);
        }
        else {
            // piggy-backing mask after the b's
            nValues++;
            bits = &(this->generator.b_padded_bits);
        }
        this->mac_vole->evaluate(this->macs[j], nValues, *bits);
    }
#ifdef OTCORR_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tCorrelated OT time: " << elapsed/1000000 << endl << flush;
#endif
}

template<>
void OTMultiplier<gfp, gfp, gfp, square128>::multiplyForBits()
{
    multiplyForTriples();
}

template<>
void OTMultiplier<gf2n, gf2n, gf2n, square128>::multiplyForBits()
{
    int nBits = generator.nTriplesPerLoop + generator.field_size;
    int nBlocks = ceil(1.0 * nBits / generator.field_size);
    OTExtensionWithMatrix auth_ot_ext(128, 128, 0, 1,
            generator.players[thread_num], keyBits, senderOutput,
            receiverOutput, BOTH, true);
    auth_ot_ext.resize(nBlocks * generator.field_size);
    macs.resize(1);
    macs[0].resize(nBits);

    pthread_mutex_lock(&mutex);
    signal_generator();
    wait_for_generator();

    for (int i = 0; i < generator.nloops; i++)
    {
        auth_ot_ext.expand<gf2n>(0, nBlocks);
        auth_ot_ext.correlate<gf2n>(0, nBlocks, generator.valueBits[0], true);
        auth_ot_ext.transpose(0, nBlocks);

        for (int j = 0; j < nBits; j++)
        {
            int128 r = auth_ot_ext.receiverOutputMatrix.squares[j/128].rows[j%128];
            int128 s = auth_ot_ext.senderOutputMatrices[0].squares[j/128].rows[j%128];
            macs[0][j] = r ^ s;
        }

        signal_generator();
        wait_for_generator();
    }

    pthread_mutex_unlock(&mutex);
}

template<class T, class U, class V, class X>
void OTMultiplier<T, U, V, X>::multiplyForBits()
{
    throw runtime_error("bit generation not implemented in this case");
}

template class OTMultiplier<gf2n, gf2n, gf2n, square128>;
template class OTMultiplier<gfp, gfp, gfp, square128>;
template class MascotMultiplier<gf2n>;
template class MascotMultiplier<gfp>;

#define X(K, S) \
        template class Spdz2kMultiplier<K, S>; \
        template class OTMultiplier<Z2<K+S>, Z2<S>, Z2<K+2*S>, Z2kRectangle<TAU(K,S),K+S> >;
X(64, 96)
X(64, 64)
X(32, 32)
