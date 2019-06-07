/*
 * Input.h
 *
 */

#ifndef PROCESSOR_INPUT_H_
#define PROCESSOR_INPUT_H_

#include <vector>
using namespace std;

#include "Tools/Buffer.h"
#include "Tools/time-func.h"
#include "Tools/PointerVector.h"

class ArithmeticProcessor;

template<class T>
class InputBase
{
    typedef typename T::clear clear;

    Player* P;

protected:
    Buffer<typename T::clear, typename T::clear> buffer;
    Timer timer;

    vector<octetStream> os;

public:
    int values_input;

    static void input(SubProcessor<T>& Proc, const vector<int>& args);

    InputBase(ArithmeticProcessor* proc);
    virtual ~InputBase();

    virtual void reset(int player) = 0;
    void reset_all(Player& P);

    virtual void add_mine(const typename T::open_type& input) = 0;
    virtual void add_other(int player) = 0;
    void add_from_all(const clear& input);

    virtual void send_mine() = 0;
    virtual void exchange();

    virtual T finalize_mine() = 0;
    virtual void finalize_other(int player, T& target, octetStream& o) = 0;
    T finalize(int player);
};

template<class T>
class Input : public InputBase<T>
{
    typedef typename T::open_type open_type;
    typedef typename T::clear clear;
    typedef typename T::MAC_Check MAC_Check;

    SubProcessor<T>& proc;
    MAC_Check& MC;
    vector< PointerVector<T> > shares;
    open_type rr, t, xi;

public:
    Input(SubProcessor<T>& proc, MAC_Check& mc);
    Input(SubProcessor<T>* proc, Player& P);

    void reset(int player);

    void add_mine(const open_type& input);
    void add_other(int player);

    void send_mine();

    T finalize_mine();
    void finalize_other(int player, T& target, octetStream& o);

    void start(int player, int n_inputs);
    void stop(int player, const vector<int>& targets);
};

#endif /* PROCESSOR_INPUT_H_ */
