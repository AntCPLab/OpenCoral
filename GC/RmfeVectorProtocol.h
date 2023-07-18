/*
 * RmfeVectorProtocol.h
 *
 */

#ifndef GC_RMFEVECTORPROTOCOL_H_
#define GC_RMFEVECTORPROTOCOL_H_

#include "Protocols/Replicated.h"

namespace GC
{

template<class T>
class RmfeVectorProtocol : public ProtocolBase<T>
{
    typename T::part_type::Protocol part_protocol;

    deque<int> mul_sizes;

public:
    Player& P;

    RmfeVectorProtocol(Player& P);

    void init(Preprocessing<T>& prep, typename T::MAC_Check& MC);

    void init_mul();
    void prepare_mul(const T& x, const T& y, int n = -1);
    void exchange();
    void finalize_mult(T& res, int n = -1);
    T finalize_mul(int n = -1);

    typename T::part_type::Protocol& get_part()
    {
        return part_protocol;
    }
};

} /* namespace GC */

#endif /* GC_RMFEVECTORPROTOCOL_H_ */
