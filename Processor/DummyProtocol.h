/*
 * DummyProtocol.h
 *
 */

#ifndef PROCESSOR_DUMMYPROTOCOL_H_
#define PROCESSOR_DUMMYPROTOCOL_H_

#include <vector>
using namespace std;

#include "Math/BitVec.h"

class Player;

template<class T> class SubProcessor;

class DummyMC
{
public:
    void Check(Player& P)
    {
        (void) P;
    }
};

class DummyProtocol
{
public:
    DummyProtocol(Player& P)
    {
        (void) P;
    }
};

template<class V>
class NotImplementedInput
{
public:
    template<class T, class U>
    NotImplementedInput(T& proc, U& MC)
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
    void add_mine(int a, int b)
    {
        (void) a, (void) b;
        throw not_implemented();
    }
    void exchange()
    {
        throw not_implemented();
    }
    V finalize(int a, int b)
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
