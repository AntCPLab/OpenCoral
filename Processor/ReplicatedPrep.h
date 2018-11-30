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
    static const int buffer_size = 1000;

    vector<array<T, 3>> triples;
    vector<array<T, 2>> squares;
    vector<array<T, 2>> inverses;
    vector<T> bits;

    virtual void buffer_triples() = 0;
    virtual void buffer_squares() = 0;
    virtual void buffer_inverses() = 0;
    virtual void buffer_bits() = 0;

    virtual void buffer_inverses(MAC_Check_Base<T>& MC, Player& P);

public:
    virtual ~BufferPrep() {}

    void get_three(Dtype dtype, T& a, T& b, T& c);
    void get_two(Dtype dtype, T& a, T& b);
    void get_one(Dtype dtype, T& a);
    void get_input(T& a, typename T::clear& x, int i);
    void get(vector<T>& S, DataTag tag, const vector<int>& regs,
            int vector_size);
};

template<class T>
class ReplicatedPrep : public BufferPrep<T>
{
    template<class U> friend class MaliciousRepPrep;

    Replicated<T>* protocol;

    void buffer_triples();
    void buffer_squares();
    void buffer_inverses();
    void buffer_bits();

public:
    ReplicatedPrep();

    void set_protocol(Replicated<T>& protocol) { this->protocol = &protocol; }
};

#endif /* PROCESSOR_REPLICATEDPREP_H_ */
