
#ifndef _OTVOLE
#define _OTVOLE

#include "Math/Z2k.h"
#include "OTExtension.h"
#include "Row.h"

using namespace std;

template <class T, class U>
class OTVoleBase : public OTExtension
{
public:
	static const int S = U::N_BITS;

	OTVoleBase(int nbaseOTs, int baseLength,
	                int nloops, int nsubloops,
	                TwoPartyPlayer* player,
	                const BitVector& baseReceiverInput,
	                const vector< vector<BitVector> >& baseSenderInput,
	                const vector<BitVector>& baseReceiverOutput,
	                OT_ROLE role=BOTH,
	                bool passive=false)
	    : OTExtension(nbaseOTs, baseLength, nloops, nsubloops, player, baseReceiverInput,
	            baseSenderInput, baseReceiverOutput, INV_ROLE(role), passive),
	        corr_prime(),
	        t0(U::N_BITS),
	        t1(U::N_BITS),
	        u(U::N_BITS),
	        t(U::N_BITS),
	        a(U::N_BITS) {
	            // need to flip roles for OT extension init, reset to original role here
	            this->ot_role = role;
	            local_prng.ReSeed();
	        }

	    void evaluate(vector<T>& output, const vector<T>& newReceiverInput);

	    void evaluate(vector<T>& output, int nValues, const BitVector& newReceiverInput);

	protected:

		// Sender fields
		Row<T> corr_prime;
		vector<Row<T>> t0, t1;
		// Receiver fields
		vector<Row<T>> u, t, a;
		// Both
		PRNG local_prng;

		Row<T> tmp;

	    virtual void consistency_check (vector<octetStream>& os);

	    void set_coeffs(__m128i* coefficients, PRNG& G, int num_elements) const;

	    void hash_row(octetStream& os, const Row<T>& row, const __m128i* coefficients);

	    void hash_row(octet* hash, const Row<T>& row, const __m128i* coefficients);

};

template <class T, class U>
class OTVole : public OTVoleBase<T, U>
{

public:
    OTVole(int nbaseOTs, int baseLength,
                int nloops, int nsubloops,
                TwoPartyPlayer* player,
                const BitVector& baseReceiverInput,
                const vector< vector<BitVector> >& baseSenderInput,
                const vector<BitVector>& baseReceiverOutput,
                OT_ROLE role=BOTH,
                bool passive=false)
    : OTVoleBase<T, U>(nbaseOTs, baseLength, nloops, nsubloops, player, baseReceiverInput,
            baseSenderInput, baseReceiverOutput, INV_ROLE(role), passive) {
    }

protected:

    Row<T> tmp;

    void consistency_check(vector<octetStream>& os);

};

#endif
