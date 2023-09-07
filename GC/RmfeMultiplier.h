#ifndef GC_RMFEMULTIPLIER_H_
#define GC_RMFEMULTIPLIER_H_

#include "OT/OTMultiplier.h"
#include "Networking/EmpChannel.h"
#include "SilentOT.h"

/**
 * Functionality OLE Implementation
*/
template<class T>
class Fole {
    const int threads = 1;
    OT_ROLE role;
    TwoPartyPlayer* player;
    EmpChannel** ios;
    /* [zico] The same Ferret instance cannot be used as sender and receiver at the same time, so we need two instances here. */
    SilentOT<EmpChannel>* ot;
    SilentOT<EmpChannel>* ot_reversed;
    BitVector keyBits;
public:

    Fole(TwoPartyPlayer* player, OT_ROLE role=BOTH, bool passive=false, int thread_num = -1);
    ~Fole();
    void init(const BitVector& keyBits);
    void set_role(OT_ROLE role);
    OT_ROLE get_role() { return role; }
    /**
     * Input (and Output) is viewed as M x N matrix, where:
     * - M is the same as the number of `keyBits`
     * - N is the message bit length
     * 
     * Therefore, the vector size should be ceil(M/128) because 128 is the unit length. And the BitMatrix vertical length should
     * be equal to N.
    */
    void correlate(vector<BitMatrix>& output, const vector<BitMatrix>& senderInput, int n);
};

template <class T>
class RmfeMultiplier : public OTMultiplier<T>
{
    OTCorrelator<Matrix<typename T::Square> > auth_ot_ext;
    void after_correlation();
    void init_authenticator(const BitVector& baseReceiverInput,
            const vector< array<BitVector, 2> >& baseSenderInput,
            const vector<BitVector>& baseReceiverOutput);

public:

    RmfeMultiplier(OTTripleGenerator<T>& generator, int thread_num);

	void multiplyForInputs(MultJob job);
};

#endif