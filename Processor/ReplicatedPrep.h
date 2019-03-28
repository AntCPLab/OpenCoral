/*
 * ReplicatedPrep.h
 *
 */

#ifndef PROCESSOR_REPLICATEDPREP_H_
#define PROCESSOR_REPLICATEDPREP_H_

#include "Networking/Player.h"
#include "Data_Files.h"

#include <array>

template<class T>
class BufferPrep : public Preprocessing<T>
{
protected:
    vector<array<T, 3>> triples;
    vector<array<T, 2>> squares;
    vector<array<T, 2>> inverses;
    vector<T> bits;
    vector<vector<InputTuple<T>>> inputs;

    virtual void buffer_triples() = 0;
    virtual void buffer_squares() = 0;
    virtual void buffer_inverses() = 0;
    virtual void buffer_bits() = 0;
    virtual void buffer_inputs(int player);

    virtual void buffer_inverses(MAC_Check_Base<T>& MC, Player& P);

public:
    int buffer_size;

    BufferPrep(DataPositions& usage) : Preprocessing<T>(usage), buffer_size(10000) {}
    virtual ~BufferPrep() {}

    void get_three_no_count(Dtype dtype, T& a, T& b, T& c);
    void get_two_no_count(Dtype dtype, T& a, T& b);
    void get_one_no_count(Dtype dtype, T& a);
    void get_input_no_count(T& a, typename T::open_type& x, int i);
    void get_no_count(vector<T>& S, DataTag tag, const vector<int>& regs,
            int vector_size);
};

template<class T>
class RingPrep : public BufferPrep<T>
{
protected:
    template<class U> friend class MaliciousRepPrep;

    typename T::Protocol* protocol;
    SubProcessor<T>* proc;

    int base_player;

    void buffer_squares();
    void buffer_inverses() { throw runtime_error("not inverses in rings"); }

public:
    RingPrep(SubProcessor<T>* proc, DataPositions& usage);
    virtual ~RingPrep() {}

    void set_protocol(typename T::Protocol& protocol);

    virtual void buffer_bits();
};

template<class T>
class ReplicatedRingPrep : public RingPrep<T>
{
protected:
    void buffer_triples();
    void buffer_squares();

public:
    ReplicatedRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
        RingPrep<T>(proc, usage) {};
};

template<class T>
class ReplicatedPrep: public ReplicatedRingPrep<T>
{
    void buffer_inverses();

public:
    ReplicatedPrep(SubProcessor<T>* proc, DataPositions& usage) : ReplicatedRingPrep<T>(proc, usage) {}

    void buffer_bits();
};

#endif /* PROCESSOR_REPLICATEDPREP_H_ */
