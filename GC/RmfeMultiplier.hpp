#ifndef GC_RMFEMULTIPLIER_HPP_
#define GC_RMFEMULTIPLIER_HPP_

#include "GC/RmfeMultiplier.h"

template<class T>
void RmfeMultiplier<T>::after_correlation() {
    throw runtime_error("not implemented for rmfe multiplier");
}

template<class T>
void RmfeMultiplier<T>::init_authenticator(const BitVector& keyBits,
		const vector< array<BitVector, 2> >& senderOutput,
		const vector<BitVector>& receiverOutput) {
    this->auth_ot_ext.init(keyBits, senderOutput, receiverOutput);
}

template<class T>
RmfeMultiplier<T>::RmfeMultiplier(OTTripleGenerator<T>& generator, int thread_num) :
        OTMultiplier<T>(generator, thread_num),
    auth_ot_ext(generator.players[thread_num], BOTH, true) {
}

template<class T>
void RmfeMultiplier<T>::multiplyForInputs(MultJob job) {
    // [TODO] Replace with our OLE through usage of MFE. Now it is just a copy of MascotMultiplier.
    
    assert(job.input);
    auto& generator = this->generator;
    bool mine = job.player == generator.my_num;
    auth_ot_ext.set_role(mine ? RECEIVER : SENDER);
    int nOTs = job.n_inputs * generator.field_size;
    auth_ot_ext.resize(nOTs);
    auth_ot_ext.expand(0, job.n_inputs);
    if (mine)
        this->inbox.pop();
    {
        print_total_comm(generator.get_player(), "expand");
    }

    auth_ot_ext.correlate(0, job.n_inputs, generator.valueBits[0], true);
    {
        print_total_comm(generator.get_player(), "correlate");
    }
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

#endif