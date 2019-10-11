/*
 * ReplicatedPrep.h
 *
 */

#ifndef PROTOCOLS_REPLICATEDPREP_H_
#define PROTOCOLS_REPLICATEDPREP_H_

#include "Networking/Player.h"
#include "Processor/Data_Files.h"
#include "Protocols/Rep3Share.h"
#include "Math/gfp.h"

#include <array>

template<class T>
void buffer_inverses(vector<array<T, 2>>& inverses, Preprocessing<T>& prep,
        MAC_Check_Base<T>& MC, Player& P);

template<class T>
class BufferPrep : public Preprocessing<T>
{
protected:
    vector<array<T, 3>> triples;
    vector<array<T, 2>> squares;
    vector<array<T, 2>> inverses;
    vector<T> bits;
    vector<vector<InputTuple<T>>> inputs;

    int n_bit_rounds;

    virtual void buffer_triples() = 0;
    virtual void buffer_squares() = 0;
    virtual void buffer_inverses() = 0;
    virtual void buffer_bits() = 0;
    virtual void buffer_inputs(int player);

    // don't call this if T::Input requires input tuples
    void buffer_inputs_as_usual(int player, SubProcessor<T>* proc);

public:
    typedef T share_type;

    int buffer_size;

    static void basic_setup(Player& P) { (void) P; }
    static void setup(Player& P, typename T::mac_key_type alphai) { (void) P, (void) alphai; }
    static void teardown() {}

    BufferPrep(DataPositions& usage);
    virtual ~BufferPrep();

    void clear();

    void get_three_no_count(Dtype dtype, T& a, T& b, T& c);
    void get_two_no_count(Dtype dtype, T& a, T& b);
    void get_one_no_count(Dtype dtype, T& a);
    void get_input_no_count(T& a, typename T::open_type& x, int i);
    void get_no_count(vector<T>& S, DataTag tag, const vector<int>& regs,
            int vector_size);

    T get_random_from_inputs(int nplayers);
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

    void buffer_bits_without_check();

public:
    RingPrep(SubProcessor<T>* proc, DataPositions& usage);
    virtual ~RingPrep() {}

    void set_proc(SubProcessor<T>* proc) { this->proc = proc; }
    SubProcessor<T>* get_proc() { return proc; }
    vector<T>& get_bits() { return this->bits; }

    void set_protocol(typename T::Protocol& protocol);
};

template<class T>
class SemiHonestRingPrep : public virtual RingPrep<T>
{
public:
	SemiHonestRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
		RingPrep<T>(proc, usage) {}
    virtual ~SemiHonestRingPrep() {}

    virtual void buffer_bits() { this->buffer_bits_without_check(); }
    virtual void buffer_inputs(int player)
    { this->buffer_inputs_as_usual(player, this->proc); }
};

template<class T>
class MaliciousRingPrep : public RingPrep<T>
{
public:
    MaliciousRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
        RingPrep<T>(proc, usage) {}
    virtual ~MaliciousRingPrep() {}

    virtual void buffer_bits();
};

template<class T>
class ReplicatedRingPrep : public SemiHonestRingPrep<T>
{
protected:
    void buffer_triples();
    void buffer_squares();

public:
    ReplicatedRingPrep(SubProcessor<T>* proc, DataPositions& usage) :
        RingPrep<T>(proc, usage), SemiHonestRingPrep<T>(proc, usage) {};
};

template<class T>
class ReplicatedPrep: public ReplicatedRingPrep<T>
{
    void buffer_inverses();

public:
    ReplicatedPrep(SubProcessor<T>* proc, DataPositions& usage) :
            RingPrep<T>(proc, usage), ReplicatedRingPrep<T>(proc, usage)
    {
    }

    void buffer_bits();
};

#endif /* PROTOCOLS_REPLICATEDPREP_H_ */
