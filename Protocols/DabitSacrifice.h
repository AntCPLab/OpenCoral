/*
 * DabitSacrifice.h
 *
 */

#ifndef PROTOCOLS_DABITSACRIFICE_H_
#define PROTOCOLS_DABITSACRIFICE_H_

#include "Processor/BaseMachine.h"

template<class T>
class DabitSacrifice
{
    const int S;

    size_t n_masks, n_produced;

public:
    DabitSacrifice();
    ~DabitSacrifice();

    int minimum_n_inputs(int n_outputs = 0, int force_packing = 1)
    {
        if (T::clear::N_BITS < 0)
            // sacrifice uses S^2 random bits
            n_outputs = BaseMachine::batch_size<T>(DATA_DABIT,
                    n_outputs, max(n_outputs, 10 * S * S));
        else
            n_outputs = BaseMachine::batch_size<T>(DATA_DABIT, n_outputs);
        assert(n_outputs > 0);
        return DIV_CEIL(n_outputs, force_packing) * force_packing + DIV_CEIL(S, force_packing) * force_packing;
    }

    void sacrifice_without_bit_check(vector<dabit<T>>& dabits,
            vector<dabit<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);

    void sacrifice_and_check_bits(vector<dabit<T>>& dabits,
            vector<dabit<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);

    void sacrifice_without_bit_check(vector<dabitpack<T>>& dabits,
        vector<dabitpack<T>>& check_dabits, SubProcessor<T>& proc,
        ThreadQueues* queues = 0);

    void sacrifice_and_check_bits(vector<dabitpack<T>>& dabits,
            vector<dabitpack<T>>& check_dabits, SubProcessor<T>& proc,
            ThreadQueues* queues = 0);
};

#endif /* PROTOCOLS_DABITSACRIFICE_H_ */
