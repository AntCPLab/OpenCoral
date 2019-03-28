/*
 * Input.h
 *
 */

#ifndef PROCESSOR_INPUT_H_
#define PROCESSOR_INPUT_H_

#include <vector>
using namespace std;

#include "Math/Share.h"
#include "Tools/Buffer.h"
#include "Tools/time-func.h"
#include "Tools/PointerVector.h"
#include "Auth/MAC_Check.h"

class ArithmeticProcessor;

template<class T>
class InputBase
{
protected:
    Buffer<typename T::clear, typename T::clear> buffer;
    Timer timer;

public:
    int values_input;

    static void input(SubProcessor<T>& Proc, const vector<int>& args);

    InputBase(ArithmeticProcessor* proc);
    ~InputBase();
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
    octetStream o;
    open_type rr, t, xi;

    void adjust_mac(T& share, const open_type& value);

public:
    Input(SubProcessor<T>& proc, MAC_Check& mc);
    Input(SubProcessor<T>* proc, Player& P);

    void reset(int player);
    void add_mine(const clear& input);
    void add_other(int player);
    void send_mine();

    T finalize_mine();
    void finalize_other(int player, T& target, octetStream& o);

    void start(int player, int n_inputs);
    void stop(int player, const vector<int>& targets);
};

#endif /* PROCESSOR_INPUT_H_ */
