/*
 * DummyProtocol.h
 *
 */

#ifndef PROCESSOR_DUMMYPROTOCOL_H_
#define PROCESSOR_DUMMYPROTOCOL_H_

#include <vector>
using namespace std;

#include "Math/BitVec.h"
#include "Data_Files.h"

class Player;
class DataPositions;
class ThreadQueues;

template<class T> class SubProcessor;

namespace GC
{
class NoShare;

template<class T> class ShareThread;
}

template<class T>
class DummyMC
{
public:
    void POpen(vector<typename T::open_type>&, vector<T>&, Player&)
    {
        throw not_implemented();
    }

    void Check(Player& P)
    {
        (void) P;
    }

    DummyMC<typename T::part_type>& get_part_MC()
    {
        return *new DummyMC<typename T::part_type>;
    }

    typename T::mac_key_type get_alphai()
    {
        throw not_implemented();
        return {};
    }
};

class DummyProtocol
{
public:
    Player& P;

    static int get_n_relevant_players()
    {
        throw not_implemented();
    }

    DummyProtocol(Player& P) :
            P(P)
    {
    }

    template<class T>
    void init_mul(SubProcessor<T>* = 0)
    {
        throw not_implemented();
    }
    template<class T>
    void prepare_mul(const T&, const T&, int = 0)
    {
        throw not_implemented();
    }
    void exchange()
    {
        throw not_implemented();
    }
    int finalize_mul(int = 0)
    {
        throw not_implemented();
        return 0;
    }
};

template<class T>
class DummyLivePrep : public Preprocessing<T>
{
public:
    static void fail()
    {
        throw runtime_error(
                "live preprocessing not implemented for " + T::type_string());
    }

    DummyLivePrep(DataPositions& usage, GC::ShareThread<T>&) :
            Preprocessing<T>(usage)
    {
    }
    DummyLivePrep(DataPositions& usage, bool = true) :
            Preprocessing<T>(usage)
    {
    }

    void set_protocol(typename T::Protocol&)
    {
    }
    void get_three_no_count(Dtype, T&, T&, T&)
    {
        fail();
    }
    void get_two_no_count(Dtype, T&, T&)
    {
        fail();
    }
    void get_one_no_count(Dtype, T&)
    {
        fail();
    }
    void get_input_no_count(T&, typename T::open_type&, int)
    {
        fail();
    }
    void get_no_count(vector<T>&, DataTag, const vector<int>&, int)
    {
        fail();
    }
    void buffer_personal_triples(vector<array<T, 3>>&, size_t, size_t)
    {
        fail();
    }
    void buffer_personal_triples(size_t, ThreadQueues*)
    {
        fail();
    }
    void shrink_to_fit()
    {
        fail();
    }
};

template<class V>
class NotImplementedInput
{
public:
    template<class T, class U>
    NotImplementedInput(const T& proc, const U& MC)
    {
        (void) proc, (void) MC;
    }
    NotImplementedInput(Player& P)
    {
        (void) P;
    }
    void start(int n, vector<int> regs)
    {
        (void) n, (void) regs;
        throw not_implemented();
    }
    void stop(int n, vector<int> regs)
    {
        (void) n, (void) regs;
        throw not_implemented();
    }
    void start(int n, int m)
    {
        (void) n, (void) m;
        throw not_implemented();
    }
    void stop(int n, int m)
    {
        (void) n, (void) m;
        throw not_implemented();
    }
    template<class T>
    static void input(SubProcessor<T>& proc, vector<int> regs)
    {
        (void) proc, (void) regs;
        throw not_implemented();
    }
    void reset_all(Player& P)
    {
        (void) P;
        throw not_implemented();
    }
    template<class U>
    void add_mine(U a, int b = 0)
    {
        (void) a, (void) b;
        throw not_implemented();
    }
    void add_other(int)
    {
        throw not_implemented();
    }
    void exchange()
    {
        throw not_implemented();
    }
    V finalize(int a, int b = 0)
    {
        (void) a, (void) b;
        throw not_implemented();
    }
};

class NotImplementedOutput
{
public:
    template<class T>
    NotImplementedOutput(SubProcessor<T>& proc)
    {
        (void) proc;
    }

    void start(int player, int target, int source)
    {
        (void) player, (void) target, (void) source;
        throw not_implemented();
    }
    void stop(int player, int source)
    {
        (void) player, (void) source;
    }
};

#endif /* PROCESSOR_DUMMYPROTOCOL_H_ */
