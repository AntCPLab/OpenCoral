#ifndef GC_RMFEMULTIPLIER_H_
#define GC_RMFEMULTIPLIER_H_

#include "OT/OTMultiplier.h"

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