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

template<class T, class U, class V, class W, class X>
OTMultiplier<T, U, V, W, X>::OTMultiplier(NPartyTripleGenerator& generator,
        int thread_num) :
        generator(generator), thread_num(thread_num),
        rot_ext(128, 128, 0, 1,
                generator.players[thread_num], generator.baseReceiverInput,
                generator.baseSenderInputs[thread_num],
                generator.baseReceiverOutputs[thread_num], BOTH, !generator.machine.check),
        auth_ot_ext(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, true),
        otCorrelator(0, 0, 0, 0, generator.players[thread_num], {}, {}, {}, BOTH, true)
{
    pthread_mutex_init(&mutex, 0);
    pthread_cond_init(&ready, 0);
    thread = 0;
}

template<class T>
MascotMultiplier<T>::MascotMultiplier(NPartyTripleGenerator& generator,
        int thread_num) :
        OTMultiplier<T, T, T, square128, square128>(generator, thread_num)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template<class T, class U, class V, class W, class X>
OTMultiplier<T, U, V, W, X>::~OTMultiplier()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&ready);
}

template<class T, class U, class V, class W, class X>
void OTMultiplier<T, U, V, W, X>::multiply()
{
    keyBits.set(generator.machine.get_mac_key<U>());
    rot_ext.extend<T>(keyBits.size(), keyBits);
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
    auth_ot_ext.init(keyBits, senderOutput, receiverOutput);

    if (generator.machine.generateBits)
    	multiplyForBits();
    else
    	multiplyForTriples();
}

template<class T, class U, class V, class W, class X>
void OTMultiplier<T, U, V, W, X>::multiplyForTriples()
{
    auth_ot_ext.resize(generator.nPreampTriplesPerLoop * W::N_COLUMNS);

    // dummy input for OT correlator
    vector<BitVector> _;
    vector< vector<BitVector> > __;
    BitVector ___;

    otCorrelator.resize(X::N_COLUMNS * generator.nPreampTriplesPerLoop);

    rot_ext.resize(X::N_ROWS * generator.nPreampTriplesPerLoop + 2 * 128);
    
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&ready);
    pthread_cond_wait(&ready, &mutex);

    for (int i = 0; i < generator.nloops; i++)
    {
        BitVector aBits = generator.valueBits[0];
        //timers["Extension"].start();
        rot_ext.extend<T>(X::N_ROWS * generator.nPreampTriplesPerLoop, aBits);
        //timers["Extension"].stop();

        //timers["Correlation"].start();
        otCorrelator.setup_for_correlation(aBits, rot_ext.senderOutputMatrices,
                rot_ext.receiverOutputMatrix);
        otCorrelator.template correlate<T>(0, generator.nPreampTriplesPerLoop,
                generator.valueBits[1], false, generator.nAmplify);
        //timers["Correlation"].stop();

        //timers["Triple computation"].start();

        this->after_correlation();

        pthread_cond_signal(&this->ready);
        pthread_cond_wait(&this->ready, &this->mutex);
    }

    pthread_mutex_unlock(&mutex);
}

template <class T>
void MascotMultiplier<T>::after_correlation()
{
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
}

template<>
void OTMultiplier<gfp, gfp, gfp, square128, square128>::multiplyForBits()
{
	multiplyForTriples();
}

template<>
void OTMultiplier<gf2n, gf2n, gf2n, square128, square128>::multiplyForBits()
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
    pthread_cond_signal(&ready);
    pthread_cond_wait(&ready, &mutex);

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

        pthread_cond_signal(&ready);
        pthread_cond_wait(&ready, &mutex);
    }

    pthread_mutex_unlock(&mutex);
}

template<class T, class U, class V, class W, class X>
void OTMultiplier<T, U, V, W, X>::multiplyForBits()
{
    throw runtime_error("bit generation not implemented in this case");
}

template class OTMultiplier<gf2n, gf2n, gf2n, square128, square128>;
template class OTMultiplier<gfp, gfp, gfp, square128, square128>;
template class MascotMultiplier<gf2n>;
template class MascotMultiplier<gfp>;

#define X(K, S) \
        template class Spdz2kMultiplier<K, S>; \
        template class OTMultiplier<Z2<K+S>, Z2<S>, Z2<K+S>, Z2kRectangle<K+S,K+S>, Z2kRectangle<TAU(K,S),K+S> >;
X(64, 96)
