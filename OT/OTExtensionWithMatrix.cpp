/*
 * OTExtensionWithMatrix.cpp
 *
 */

#include "OTExtensionWithMatrix.h"
#include "Rectangle.h"
#include "Math/gfp.h"
#include "Math/Z2k.h"
#include "Math/gf2nlong.h"
#include "Math/BitVec.h"
#include "GC/TinySecret.h"

#include "OT/Rectangle.hpp"
#include "Math/Z2k.hpp"
#include "Math/Square.hpp"

OTExtensionWithMatrix OTExtensionWithMatrix::setup(TwoPartyPlayer& player,
        int128 delta, OT_ROLE role, bool passive)
{
    BaseOT baseOT(128, 128, &player, INV_ROLE(role));
    PRNG G;
    G.ReSeed();
    baseOT.set_receiver_inputs(delta);
    baseOT.exec_base(false);
    return OTExtensionWithMatrix(baseOT, &player, passive);
}

OTExtensionWithMatrix::OTExtensionWithMatrix(BaseOT& baseOT, TwoPartyPlayer* player,
        bool passive) : OTCorrelator(baseOT, player, passive)
{
    G.ReSeed();
}

void OTExtensionWithMatrix::seed(vector<BitMatrix>& baseSenderInput,
        BitMatrix& baseReceiverOutput)
{
    nbaseOTs = baseReceiverInput.size();
    //cout << "nbaseOTs " << nbaseOTs << endl;
    G_sender.resize(nbaseOTs, vector<PRNG>(2));
    G_receiver.resize(nbaseOTs);

    // set up PRGs for expanding the seed OTs
    for (int i = 0; i < nbaseOTs; i++)
    {
        if (ot_role & RECEIVER)
        {
            G_sender[i][0].SetSeed((octet*)&baseSenderInput[0].squares[i/128].rows[i%128]);
            G_sender[i][1].SetSeed((octet*)&baseSenderInput[1].squares[i/128].rows[i%128]);
        }
        if (ot_role & SENDER)
        {
            G_receiver[i].SetSeed((octet*)&baseReceiverOutput.squares[i/128].rows[i%128]);
        }
    }
}

void OTExtensionWithMatrix::transfer(int nOTs,
        const BitVector& receiverInput)
{
#ifdef OTEXT_TIMER
    timeval totalstartv, totalendv;
    gettimeofday(&totalstartv, NULL);
#endif
    cout << "\tDoing " << nOTs << " extended OTs as " << role_to_str(ot_role) << endl;

    // resize to account for extra k OTs that are discarded
    BitVector newReceiverInput(nOTs);
    for (unsigned int i = 0; i < receiverInput.size_bytes(); i++)
    {
        newReceiverInput.set_byte(i, receiverInput.get_byte(i));
    }

    for (int loop = 0; loop < nloops; loop++)
    {
        extend(nOTs, newReceiverInput);
#ifdef OTEXT_TIMER
        gettimeofday(&totalendv, NULL);
        double elapsed = timeval_diff(&totalstartv, &totalendv);
        cout << "\t\tTotal thread time: " << elapsed/1000000 << endl << flush;
#endif
    }

#ifdef OTEXT_TIMER
    gettimeofday(&totalendv, NULL);
    times["Total thread"] +=  timeval_diff(&totalstartv, &totalendv);
#endif
}

template <class U>
void OTCorrelator<U>::resize(int nOTs)
{
    t1.resize_vertical(nOTs);
    u.resize_vertical(nOTs);
    senderOutputMatrices.resize(2);
    for (int i = 0; i < 2; i++)
        senderOutputMatrices[i].resize_vertical(nOTs);
    receiverOutputMatrix.resize_vertical(nOTs);
}

// the template is used to denote the field of the hash output
void OTExtensionWithMatrix::extend(int nOTs_requested, BitVector& newReceiverInput)
{
    extend_correlated(nOTs_requested, newReceiverInput);
    hash_outputs(nOTs_requested);
}

void OTExtensionWithMatrix::extend_correlated(const BitVector& newReceiverInput)
{
    extend_correlated(newReceiverInput.size(), newReceiverInput);
}

void OTExtensionWithMatrix::extend_correlated(int nOTs_requested, const BitVector& newReceiverBits)
{
//    if (nOTs % nbaseOTs != 0)
//        throw invalid_length(); //"nOTs must be a multiple of nbaseOTs\n");
    if (nOTs_requested == 0)
        return;
    // local copy
    auto newReceiverInput = newReceiverBits;
    if ((ot_role & RECEIVER) and (size_t)nOTs_requested != newReceiverInput.size())
        throw runtime_error("wrong number of choice bits");
    int nOTs_requested_rounded = (nOTs_requested + 127) / 128 * 128;
    // add k + s to account for discarding k OTs
    int nOTs = nOTs_requested_rounded + 2 * 128;

    int slice = nOTs / nsubloops / 128;
    nOTs = slice * nsubloops * 128;
    resize(nOTs);
    newReceiverInput.resize_zero(nOTs);

    // randomize last 128 + 128 bits that will be discarded
    for (int i = 0; i < 4; i++)
        newReceiverInput.set_word(nOTs/64 - i - 1, G.get_word());

    // subloop for first part to interleave communication with computation
    for (int start = 0; start < nOTs / 128; start += slice)
    {
        expand(start, slice);
        this->correlate(start, slice, newReceiverInput, true);
        transpose(start, slice);
    }

#ifdef OTEXT_TIMER
    double elapsed;
#endif
    // correlation check
    if (!passive_only)
    {
#ifdef OTEXT_TIMER
        timeval startv, endv;
        gettimeofday(&startv, NULL);
#endif
        check_correlation(nOTs, newReceiverInput);
#ifdef OTEXT_TIMER
        gettimeofday(&endv, NULL);
        elapsed = timeval_diff(&startv, &endv);
        cout << "\t\tTotal correlation check time: " << elapsed/1000000 << endl << flush;
        times["Total correlation check"] += timeval_diff(&startv, &endv);
#endif
    }

    receiverOutputMatrix.resize(nOTs_requested_rounded);
    senderOutputMatrices[0].resize(nOTs_requested_rounded);
    senderOutputMatrices[1].resize(nOTs_requested_rounded);
    newReceiverInput.resize(nOTs_requested);
}

template <class U>
void OTCorrelator<U>::expand(int start, int slice)
{
    (void)start, (void)slice;
    Slice<U> receiverOutputSlice(receiverOutputMatrix, start, slice);
    Slice<U> senderOutputSlices[2] = {
            Slice<U>(senderOutputMatrices[0], start, slice),
            Slice<U>(senderOutputMatrices[1], start, slice)
    };
    Slice<U> t1Slice(t1, start, slice);

    // expand with PRG
    if (ot_role & RECEIVER)
    {
        for (int i = 0; i < nbaseOTs; i++)
        {
            receiverOutputSlice.randomize(i, G_sender[i][0]);
            t1Slice.randomize(i, G_sender[i][1]);
        }
    }

    if (ot_role & SENDER)
    {
        for (int i = 0; i < nbaseOTs; i++)
            // randomize base receiver output
            senderOutputSlices[0].randomize(i, G_receiver[i]);
    }
}

void OTExtensionWithMatrix::expand_transposed()
{
    for (int i = 0; i < nbaseOTs; i++)
    {
        if (ot_role & RECEIVER)
        {
            receiverOutputMatrix.squares[i/128].randomize(i % 128, G_sender[i][0]);
            t1.squares[i/128].randomize(i % 128, G_sender[i][1]);
        }
        if (ot_role & SENDER)
        {
            senderOutputMatrices[0].squares[i/128].randomize(i % 128, G_receiver[i]);
        }
    }
}

template <class U>
void OTCorrelator<U>::setup_for_correlation(BitVector& baseReceiverInput,
        vector<U>& baseSenderOutputs,
        U& baseReceiverOutput)
{
    this->baseReceiverInput = baseReceiverInput;
    receiverOutputMatrix = baseSenderOutputs[0];
    t1 = baseSenderOutputs[1];
    u.squares.resize(t1.squares.size());
    senderOutputMatrices.resize(2);
    senderOutputMatrices[0] = baseReceiverOutput;
}

template <class U>
void OTCorrelator<U>::correlate(int start, int slice,
        BitVector& newReceiverInput, bool useConstantBase, int repeat)
{
    vector<octetStream> os(2);

    Slice<U> receiverOutputSlice(receiverOutputMatrix, start, slice);
    Slice<U> senderOutputSlices[] = {
            Slice<U>(senderOutputMatrices[0], start, slice),
    };
    Slice<U> t1Slice(t1, start, slice);
    Slice<U> uSlice(u, start, slice);

    // create correlation
    if (ot_role & RECEIVER)
    {
        t1Slice.rsub(receiverOutputSlice);
        t1Slice.sub(newReceiverInput, repeat);
        t1Slice.pack(os[0]);

//        t1 = receiverOutputMatrix;
//        t1 ^= newReceiverInput;
//        receiverOutputMatrix.print_side_by_side(t1);
    }
#ifdef OTEXT_TIMER
    timeval commst1, commst2;
    gettimeofday(&commst1, NULL);
#endif
    // send t0 + t1 + x
    send_if_ot_receiver(player, os, ot_role);

    // sender adjusts using base receiver bits
    if (ot_role & SENDER)
    {
        // u = t0 + t1 + x
        uSlice.unpack(os[1]);
        senderOutputSlices[0].conditional_add(baseReceiverInput, u, !useConstantBase);
    }
#ifdef OTEXT_TIMER
    gettimeofday(&commst2, NULL);
    double commstime = timeval_diff(&commst1, &commst2);
    cout << "\t\tCommunication took time " << commstime/1000000 << endl << flush;
    times["Communication"] += timeval_diff(&commst1, &commst2);
#endif
}

void OTExtensionWithMatrix::transpose(int start, int slice)
{
    BitMatrixSlice receiverOutputSlice(receiverOutputMatrix, start, slice);
    BitMatrixSlice senderOutputSlices[2] = {
            BitMatrixSlice(senderOutputMatrices[0], start, slice),
            BitMatrixSlice(senderOutputMatrices[1], start, slice)
    };

    // transpose t0[i] onto receiverOutput and tmp (q[i]) onto senderOutput[i][0]

    //cout << "Starting matrix transpose\n" << flush << endl;
#ifdef OTEXT_TIMER
    timeval transt1, transt2;
    gettimeofday(&transt1, NULL);
#endif
    // transpose in 128-bit chunks
    if (ot_role & RECEIVER)
        receiverOutputSlice.transpose();
    if (ot_role & SENDER)
        senderOutputSlices[0].transpose();

#ifdef OTEXT_TIMER
    gettimeofday(&transt2, NULL);
    double transtime = timeval_diff(&transt1, &transt2);
    cout << "\t\tMatrix transpose took time " << transtime/1000000 << endl << flush;
    times["Matrix transpose"] += timeval_diff(&transt1, &transt2);
#endif
}

/*
 * Hash outputs to make into random OT
 */
void OTExtensionWithMatrix::hash_outputs(int nOTs)
{
    hash_outputs(nOTs, senderOutputMatrices, receiverOutputMatrix);
}

template <class V>
void OTExtensionWithMatrix::hash_outputs(int nOTs, vector<V>& senderOutput,
        V& receiverOutput, bool correlated)
{
    //cout << "Hashing... " << flush;
    octetStream os, h_os(HASH_SIZE);
    MMO mmo;
#ifdef OTEXT_TIMER
    timeval startv, endv;
    gettimeofday(&startv, NULL);
#endif

    typedef typename V::PartType::RowType T;

    int n_rows = V::PartType::N_ROWS_ALLOCATED;
    int n = (nOTs + n_rows - 1) / n_rows * V::PartType::N_ROWS;
    for (int i = 0; i < 2; i++)
        senderOutput[i].resize_vertical(n);
    receiverOutput.resize_vertical(n);

    if (nOTs % 8 != 0)
        throw runtime_error("number of OTs must be divisible by 8");

    for (int i = 0; i < nOTs; i += 8)
    {
        int i_outer_input = i / 128;
        int i_inner_input = i % 128;
        int i_outer_output = i / n_rows;
        int i_inner_output = i % n_rows;
        if (ot_role & SENDER)
        {
            int128 tmp[2][8];
            for (int j = 0; j < 8; j++)
            {
                tmp[0][j] = senderOutputMatrices[0].squares[i_outer_input].rows[i_inner_input + j];
                if (correlated)
                    tmp[1][j] = tmp[0][j] ^ baseReceiverInput.get_int128(0);
                else
                    tmp[1][j] =
                            senderOutputMatrices[1].squares[i_outer_input].rows[i_inner_input + j];
            }
            for (int j = 0; j < 2; j++)
                mmo.hashBlocks<T, 8>(
                        &senderOutput[j].squares[i_outer_output].rows[i_inner_output],
                        &tmp[j]);
        }
        if (ot_role & RECEIVER)
        {
            mmo.hashBlocks<T, 8>(
                    &receiverOutput.squares[i_outer_output].rows[i_inner_output],
                    &receiverOutputMatrix.squares[i_outer_input].rows[i_inner_input]);
        }
    }
    //cout << "done.\n";
#ifdef OTEXT_TIMER
    gettimeofday(&endv, NULL);
    double elapsed = timeval_diff(&startv, &endv);
    cout << "\t\tOT ext hashing took time " << elapsed/1000000 << endl << flush;
    times["Hashing"] += timeval_diff(&startv, &endv);
#endif
}

template <class U>
template <class T>
void OTCorrelator<U>::reduce_squares(unsigned int nTriples, vector<T>& output, int start)
{
    if (receiverOutputMatrix.squares.size() < nTriples + start)
        throw invalid_length();
    output.resize(nTriples);
    for (unsigned int j = 0; j < nTriples; j++)
    {
        receiverOutputMatrix.squares[j + start].sub(
                senderOutputMatrices[0].squares[j + start]).to(output[j]);
    }
}

template <class U>
void OTCorrelator<U>::common_seed(PRNG& G)
{
    Slice<U> t1Slice(t1, 0, t1.squares.size());
    Slice<U> uSlice(u, 0, u.squares.size());

    octetStream os;
    if (player->my_num())
    {
        t1Slice.pack(os);
        uSlice.pack(os);
    }
    else
    {
        uSlice.pack(os);
        t1Slice.pack(os);
    }
    auto hash = os.hash();
    G = PRNG(hash);
}

octet* OTExtensionWithMatrix::get_receiver_output(int i)
{
    return (octet*)&receiverOutputMatrix.squares[i/128].rows[i%128];
}

octet* OTExtensionWithMatrix::get_sender_output(int choice, int i)
{
    return (octet*)&senderOutputMatrices[choice].squares[i/128].rows[i%128];
}

void OTExtensionWithMatrix::print(BitVector& newReceiverInput, int i)
{
    if (player->my_num() == 0)
    {
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix, i);
        print_sender(senderOutputMatrices[0].squares[i], senderOutputMatrices[1].squares[i]);
    }
    else
    {
        print_sender(senderOutputMatrices[0].squares[i], senderOutputMatrices[1].squares[i]);
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix, i);
    }
}

template <class T>
void OTExtensionWithMatrix::print_receiver(BitVector& newReceiverInput, BitMatrix& matrix, int k, int offset)
{
    if (ot_role & RECEIVER)
    {
        for (int i = 0; i < 16; i++)
        {
            if (newReceiverInput.get_bit((offset + k) * 128 + i))
            {
                for (int j = 0; j < 33; j++)
                    cout << " ";
                cout << T(matrix.squares[k].rows[i]);
            }
            else
                cout << int128(matrix.squares[k].rows[i]);
            cout << endl;
        }
        cout << endl;
    }
}

void OTExtensionWithMatrix::print_sender(square128& square0, square128& square1)
{
    if (ot_role & SENDER)
    {
        for (int i = 0; i < 16; i++)
        {
            cout << int128(square0.rows[i]) << " ";
            cout << int128(square1.rows[i]) << " ";
            cout << endl;
        }
        cout << endl;
    }
}

template <class T>
void OTExtensionWithMatrix::print_post_correlate(BitVector& newReceiverInput, int j, int offset, int sender)
{
   cout << "post correlate, sender" << sender << endl;
   if (player->my_num() == sender)
   {
       T delta = newReceiverInput.get_int128(offset + j);
       for (int i = 0; i < 16; i++)
       {
           cout << (int128(receiverOutputMatrix.squares[j].rows[i]));
           cout << " ";
           cout << (T(receiverOutputMatrix.squares[j].rows[i]) - delta);
           cout << endl;
       }
       cout << endl;
   }
   else
   {
       print_receiver<T>(baseReceiverInput, senderOutputMatrices[0], j);
   }
}

void OTExtensionWithMatrix::print_pre_correlate(int i)
{
    cout << "pre correlate" << endl;
    if (player->my_num() == 0)
        print_sender(receiverOutputMatrix.squares[i], t1.squares[i]);
    else
        print_receiver<gf2n_long>(baseReceiverInput, senderOutputMatrices[0], i);
}

void OTExtensionWithMatrix::print_post_transpose(BitVector& newReceiverInput, int i, int sender)
{
    cout << "post transpose, sender " << sender << endl;
    if (player->my_num() == sender)
    {
        print_receiver<gf2n_long>(newReceiverInput, receiverOutputMatrix);
    }
    else
    {
        square128 tmp = senderOutputMatrices[0].squares[i];
        tmp ^= baseReceiverInput;
        print_sender(senderOutputMatrices[0].squares[i], tmp);
    }
}

void OTExtensionWithMatrix::print_pre_expand()
{
    cout << "pre expand" << endl;
    if (player->my_num() == 0)
    {
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 2; j++)
                cout << int128(_mm_loadu_si128((__m128i*)G_sender[i][j].get_seed())) << " ";
            cout << endl;
        }
        cout << endl;
    }
    else
    {
        for (int i = 0; i < 16; i++)
        {
            if (baseReceiverInput.get_bit(i))
            {
                for (int j = 0; j < 33; j++)
                    cout << " ";
            }
            cout << int128(_mm_loadu_si128((__m128i*)G_receiver[i].get_seed())) << endl;
        }
        cout << endl;
    }
}

template class OTCorrelator<BitMatrix>;

#define Z(BM,GF) \
template class OTCorrelator<BM>; \
template void OTCorrelator<BM>::reduce_squares<GF>(unsigned int nTriples, \
        vector<GF>& output, int);

#define ZZZZ(GF) \
template void OTExtensionWithMatrix::print_post_correlate<GF>( \
        BitVector& newReceiverInput, int j, int offset, int sender); \

#define ZZZ(GF, M) Z(M, GF) \
template void OTExtensionWithMatrix::hash_outputs(int, vector<M >&, M&, bool);

ZZZZ(gf2n_long)
ZZZ(gf2n_short, Matrix<gf2n_short_square>)
ZZZ(gf2n_long, Matrix<Square<gf2n_long>>)
ZZZ(gfp1, Matrix<Square<gfp1>>)
ZZZ(gfp3, Matrix<Square<gfp3>>)
ZZZ(BitVec, Matrix<BitDiagonal>)

#undef XX
#define XX(T,U,N,L) \
template class OTCorrelator<Matrix<Rectangle<Z2<N>, Z2<L> > > >; \
template void OTCorrelator<Matrix<Rectangle<Z2<N>, Z2<L> > > >::reduce_squares(unsigned int nTriples, \
        vector<U>& output, int); \
template void OTExtensionWithMatrix::hash_outputs(int, \
        std::vector<Matrix<Rectangle<Z2<N>, Z2<L> > >, std::allocator<Matrix<Rectangle<Z2<N>, Z2<L> > > > >&, \
        Matrix<Rectangle<Z2<N>, Z2<L> > >&, bool);

#undef X
#define X(N,L) \
template void OTCorrelator<Matrix<Rectangle<Z2<N>, Z2<L> > > >::reduce_squares(unsigned int nTriples, \
        vector<Z2kRectangle<N, L> >& output, int); \
XX(Z2<L>,Z2<N>,N,L)

//X(96, 160)
XX(SignedZ2<64>, SignedZ2<64>, 64, 64)
XX(SignedZ2<72>, SignedZ2<72>, 72, 72)

Y(64, 64)
Y(64, 48)
Y(66, 64)
Y(66, 48)
Y(32, 32)
Y(1, 40)
Y(72, 48)
Y(74, 48)
Y(72, 64)
Y(74, 64)
