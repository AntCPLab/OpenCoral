/*
 * ReplicatedInput.h
 *
 */

#ifndef PROCESSOR_REPLICATEDINPUT_H_
#define PROCESSOR_REPLICATEDINPUT_H_

#include "Input.h"

template <class T>
class PrepLessInput : public InputBase<T>
{
protected:
    SubProcessor<T>* processor;
    vector<T> shares;
    size_t i_share;

public:
    PrepLessInput(SubProcessor<T>* proc) :
            InputBase<T>(proc ? &proc->Proc : 0), processor(proc), i_share(0) {}
    virtual ~PrepLessInput() {}

    void start(int player, int n_inputs);
    void stop(int player, vector<int> targets);

    virtual void reset(int player) = 0;
    virtual void add_mine(const typename T::clear& input) = 0;
    virtual void add_other(int player) = 0;
    virtual void send_mine() = 0;
    virtual void finalize_other(int player, T& target, octetStream& o) = 0;

    T finalize_mine();
};

template <class T>
class ReplicatedInput : public PrepLessInput<T>
{
    SubProcessor<T>* proc;
    Player& P;
    vector<octetStream> os;
    SeededPRNG secure_prng;

public:
    ReplicatedInput(SubProcessor<T>& proc) :
            PrepLessInput<T>(&proc), proc(&proc), P(proc.P)
    {
        assert(T::length == 2);
    }
    ReplicatedInput(SubProcessor<T>& proc, ReplicatedMC<T>& MC) :
            ReplicatedInput(proc)
    {
        (void) MC;
    }
    ReplicatedInput(SubProcessor<T>* proc, Player& P) :
            PrepLessInput<T>(proc), proc(proc), P(P)
    {
    }

    void reset(int player);
    void add_mine(const typename T::clear& input);
    void add_other(int player);
    void send_mine();
    void finalize_other(int player, T& target, octetStream& o);
};

#endif /* PROCESSOR_REPLICATEDINPUT_H_ */
