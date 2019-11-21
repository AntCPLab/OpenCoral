/*
 * OTExtensionWithMatrix.h
 *
 */

#ifndef OT_OTEXTENSIONWITHMATRIX_H_
#define OT_OTEXTENSIONWITHMATRIX_H_

#include "OTExtension.h"
#include "BitMatrix.h"
#include "Math/gf2n.h"

template <class U>
class OTCorrelator : public OTExtension
{
public:
    vector<U> senderOutputMatrices;
    vector<U> matrices;
    U& receiverOutputMatrix;
    U& t1;
    U u;

    OTCorrelator(int nbaseOTs, int baseLength,
                int nloops, int nsubloops,
                TwoPartyPlayer* player,
                const BitVector& baseReceiverInput,
                const vector< vector<BitVector> >& baseSenderInput,
                const vector<BitVector>& baseReceiverOutput,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OTExtension(nbaseOTs, baseLength, nloops, nsubloops, player, baseReceiverInput,
            baseSenderInput, baseReceiverOutput, role, passive),
            senderOutputMatrices(2), matrices(2),
            receiverOutputMatrix(matrices[0]), t1(matrices[1]) {}

    OTCorrelator(BaseOT& baseOT, TwoPartyPlayer* player, bool passive) :
            OTExtension(baseOT, player, passive),
            senderOutputMatrices(2), matrices(2),
            receiverOutputMatrix(matrices[0]), t1(matrices[1]) {}

    void resize(int nOTs);
    void expand(int start, int slice);
    void setup_for_correlation(BitVector& baseReceiverInput,
            vector<U>& baseSenderOutputs,
            U& baseReceiverOutput);
    void correlate(int start, int slice, BitVector& newReceiverInput, bool useConstantBase, int repeat = 1);
    template <class T>
    void reduce_squares(unsigned int nTriples, vector<T>& output,
            int start = 0);
    void common_seed(PRNG& G);
};

class OTExtensionWithMatrix : public OTCorrelator<BitMatrix>
{
public:
    PRNG G;

    static OTExtensionWithMatrix setup(TwoPartyPlayer& player,
            int128 delta, OT_ROLE role, bool passive);

    OTExtensionWithMatrix(int nbaseOTs, int baseLength,
                int nloops, int nsubloops,
                TwoPartyPlayer* player,
                const BitVector& baseReceiverInput,
                const vector< vector<BitVector> >& baseSenderInput,
                const vector<BitVector>& baseReceiverOutput,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OTCorrelator<BitMatrix>(nbaseOTs, baseLength, nloops, nsubloops, player, baseReceiverInput,
            baseSenderInput, baseReceiverOutput, role, passive) {
      G.ReSeed();
    }

    OTExtensionWithMatrix(BaseOT& baseOT, TwoPartyPlayer* player, bool passive);

    void seed(vector<BitMatrix>& baseSenderInput,
            BitMatrix& baseReceiverOutput);
    void transfer(int nOTs, const BitVector& receiverInput);
    void extend(int nOTs, BitVector& newReceiverInput);
    void extend_correlated(const BitVector& newReceiverInput);
    void extend_correlated(int nOTs, const BitVector& newReceiverInput);
    void transpose(int start, int slice);
    void expand_transposed();
    template <class V>
    void hash_outputs(int nOTs, vector<V>& senderOutput, V& receiverOutput,
            bool correlated = true);

    void print(BitVector& newReceiverInput, int i = 0);
    template <class T>
    void print_receiver(BitVector& newReceiverInput, BitMatrix& matrix, int i = 0, int offset = 0);
    void print_sender(square128& square0, square128& square);
    template <class T>
    void print_post_correlate(BitVector& newReceiverInput, int i = 0, int offset = 0, int sender = 0);
    void print_pre_correlate(int i = 0);
    void print_post_transpose(BitVector& newReceiverInput, int i = 0,  int sender = 0);
    void print_pre_expand();

    octet* get_receiver_output(int i);
    octet* get_sender_output(int choice, int i);

protected:
    void hash_outputs(int nOTs);
};

#endif /* OT_OTEXTENSIONWITHMATRIX_H_ */
