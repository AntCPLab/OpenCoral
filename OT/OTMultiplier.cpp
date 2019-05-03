/*
 * OTMultiplier.cpp
 *
 */

#include "OT/config.h"

#include "OT/OTMultiplier.h"
#include "OT/NPartyTripleGenerator.h"
#include "OT/Rectangle.h"
#include "Math/Z2k.h"
#include "Math/SemiShare.h"
#include "Math/Semi2kShare.h"

#include "OT/OTVole.hpp"
#include "OT/Row.hpp"
#include "OT/Rectangle.hpp"

#include <math.h>

//#define OTCORR_TIMER

template<class T>
OTMultiplier<T>::OTMultiplier(OTTripleGenerator<T>& generator,
        int thread_num) :
        generator(generator), thread_num(thread_num),
        rot_ext(128, 128, 0, 1,
                generator.players[thread_num], generator.baseReceiverInput,
                generator.baseSenderInputs[thread_num],
                generator.baseReceiverOutputs[thread_num], BOTH, !generator.machine.check),
        otCorrelator(0, 0, 0, 0, generator.players[thread_num], {}, {}, {}, BOTH, true)
{
    this->thread = 0;
}

template<class T>
MascotMultiplier<T>::MascotMultiplier(OTTripleGenerator<Share<T>>& generator,
        int thread_num) :
        OTMultiplier<Share<T>>(generator, thread_num),
		auth_ot_ext(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, true)
{
    c_output.resize(generator.nTriplesPerLoop);
}

template <int K, int S>
Spdz2kMultiplier<K, S>::Spdz2kMultiplier(OTTripleGenerator<Spdz2kShare<K, S>>& generator, int thread_num) :
        OTMultiplier<Spdz2kShare<K, S>>
        (generator, thread_num)
{
#ifdef USE_OPT_VOLE
		mac_vole = new OTVole<Z2<MAC_BITS>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
		input_mac_vole = new OTVole<Z2<K + S>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
#else
		mac_vole = new OTVoleBase<Z2<MAC_BITS>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
		input_mac_vole = new OTVoleBase<Z2<K + S>, Z2<S>>(128, 128, 0, 1, generator.players[thread_num], {}, {}, {}, BOTH, false);
#endif
}

template<int K, int S>
Spdz2kMultiplier<K, S>::~Spdz2kMultiplier()
{
    delete mac_vole;
    delete input_mac_vole;
}

template<class T>
OTMultiplier<T>::~OTMultiplier()
{
}

template<class T>
void OTMultiplier<T>::multiply()
{
    keyBits.set(generator.machine.template get_mac_key<typename T::mac_key_type>());
    rot_ext.extend<gf2n_long>(keyBits.size(), keyBits);
    this->outbox.push({});
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

    MultJob job;
    while (this->inbox.pop(job))
    {
        if (job.input)
        {
            if (job.player == generator.my_num
                    or job.player == generator.players[thread_num]->other_player_num())
                multiplyForInputs(job);
            else
                this->outbox.push(job);
        }
        else
        {
            switch (job.type)
            {
            case DATA_BIT:
                multiplyForBits();
                break;
            case DATA_TRIPLE:
                multiplyForTriples();
                break;
            default:
                throw not_implemented();
            }
        }
    }
}

template<class W>
void OTMultiplier<W>::multiplyForTriples()
{
    typedef typename W::open_type T;
    typedef typename W::Rectangle X;

    // dummy input for OT correlator
    vector<BitVector> _;
    vector< vector<BitVector> > __;
    BitVector ___;

    otCorrelator.resize(X::N_COLUMNS * generator.nPreampTriplesPerLoop);

    rot_ext.resize(X::N_ROWS * generator.nPreampTriplesPerLoop + 2 * 128);
    
    vector<Matrix<X> >& baseSenderOutputs = otCorrelator.matrices;
    Matrix<X>& baseReceiverOutput = otCorrelator.senderOutputMatrices[0];

    MultJob job;
    auto& outbox = this->outbox;
    outbox.push(job);

    for (int i = 0; i < generator.nloops; i++)
    {
        this->inbox.pop(job);
        BitVector aBits = generator.valueBits[0];
        //timers["Extension"].start();
        rot_ext.extend_correlated(aBits);
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
    }
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
	input_mac_vole->init(keyBits, senderOutput, receiverOutput);
}

template<class T>
void SemiMultiplier<T>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nPreampTriplesPerLoop,
            this->c_output);

    this->outbox.push({});
}

template <class T>
void MascotMultiplier<T>::after_correlation()
{
	this->auth_ot_ext.resize(this->generator.nPreampTriplesPerLoop * square128::N_COLUMNS);
	this->auth_ot_ext.set_role(BOTH);

    this->otCorrelator.reduce_squares(this->generator.nPreampTriplesPerLoop,
            this->c_output);

    this->outbox.push({});

    if (this->generator.machine.generateMACs)
    {
        this->macs.resize(3);
        MultJob job;
        this->inbox.pop(job);
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
        this->outbox.push(job);
    }
}

template <int K, int S>
void Spdz2kMultiplier<K, S>::after_correlation()
{
    this->otCorrelator.reduce_squares(this->generator.nTriplesPerLoop,
            this->c_output);

    this->outbox.push({});
    this->inbox.pop();

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
    this->outbox.push({});
}

template<>
void OTMultiplier<Share<gfp1>>::multiplyForBits()
{
    multiplyForTriples();
}

template<>
void OTMultiplier<Share<gf2n>>::multiplyForBits()
{
    int nBits = generator.nTriplesPerLoop + generator.field_size;
    int nBlocks = ceil(1.0 * nBits / 128);
    BitVector extKeyBits = keyBits;
    extKeyBits.resize_zero(128);
    auto extSenderOutput = senderOutput;
    extSenderOutput.resize(128, {2, BitVector(128)});
    SeededPRNG G;
    for (auto& x : extSenderOutput)
        for (auto& y : x)
            if (y.size() < 128)
            {
                y.resize(128);
                y.randomize(G);
            }
    auto extReceiverOutput = receiverOutput;
    extReceiverOutput.resize(128, 128);
    OTExtensionWithMatrix auth_ot_ext(128, 128, 0, 1,
            generator.players[thread_num], extKeyBits, extSenderOutput,
            extReceiverOutput, BOTH, true);
    auth_ot_ext.set_role(BOTH);
    auth_ot_ext.resize(nBlocks * 128);
    macs.resize(1);
    macs[0].resize(nBits);

    MultJob job;
    outbox.push(job);

    for (int i = 0; i < generator.nloops; i++)
    {
        auth_ot_ext.expand<gf2n_long>(0, nBlocks);
        inbox.pop(job);
        auth_ot_ext.correlate<gf2n_long>(0, nBlocks, generator.valueBits[0], true);
        auth_ot_ext.transpose(0, nBlocks);

        for (int j = 0; j < nBits; j++)
        {
            int128 r = auth_ot_ext.receiverOutputMatrix.squares[j/128].rows[j%128];
            int128 s = auth_ot_ext.senderOutputMatrices[0].squares[j/128].rows[j%128];
            macs[0][j] = gf2n::cut(r ^ s);
        }

        outbox.push(job);
    }
}

template<class T>
void MascotMultiplier<T>::multiplyForInputs(MultJob job)
{
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    int nOTs = job.n_inputs * generator.field_size;
    auth_ot_ext.resize(nOTs);
    auth_ot_ext.template expand<T>(0, job.n_inputs);
    if (mine)
        this->inbox.pop();
    auth_ot_ext.template correlate<T>(0, job.n_inputs, generator.valueBits[0], true);
    auto& input_macs = this->input_macs;
    input_macs.resize(job.n_inputs);
    if (mine)
        for (int j = 0; j < job.n_inputs; j++)
            auth_ot_ext.receiverOutputMatrix.squares[j].to(input_macs[j]);
    else
        for (int j = 0; j < job.n_inputs; j++)
        {
            auth_ot_ext.senderOutputMatrices[0].squares[j].to(input_macs[j]);
            input_macs[j].negate();
        }
    this->outbox.push(job);
}

template<int K, int S>
void Spdz2kMultiplier<K, S>::multiplyForInputs(MultJob job)
{
    assert(job.input);
    bool mine = job.player == this->generator.my_num;
    input_mac_vole->set_role(mine ? SENDER : RECEIVER);
    if (mine)
        this->inbox.pop();
    input_mac_vole->evaluate(this->input_macs, job.n_inputs, this->generator.valueBits[0]);
    this->outbox.push(job);
}

template<class T>
void OTMultiplier<T>::multiplyForBits()
{
    throw runtime_error("bit generation not implemented in this case");
}

template class OTMultiplier<Share<gf2n>>;
template class OTMultiplier<Share<gfp1>>;
template class OTMultiplier<SemiShare<gf2n>>;
template class OTMultiplier<SemiShare<gfp1>>;
template class SemiMultiplier<SemiShare<gf2n>>;
template class SemiMultiplier<SemiShare<gfp1>>;
template class SemiMultiplier<Semi2kShare<64>>;
template class SemiMultiplier<Semi2kShare<72>>;
template class MascotMultiplier<gf2n>;
template class MascotMultiplier<gfp1>;

#define X(K, S) \
        template class Spdz2kMultiplier<K, S>; \
        template class OTMultiplier<Spdz2kShare<K, S>>;
X(64, 64)
X(64, 48)
X(66, 64)
X(66, 48)
X(32, 32)
