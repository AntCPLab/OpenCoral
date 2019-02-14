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
class Input : public InputBase<Share<T>>
{
    SubProcessor<Share<T>>& proc;
    MAC_Check<T>& MC;
    vector< vector< Share<T> > > shares;
    octetStream o;

    void adjust_mac(Share<T>& share, T& value);

public:
    Input(SubProcessor<Share<T>>& proc, MAC_Check<T>& mc);

    void reset(int player);
    void add_mine(const T& input);
    void add_other(int player);
    void send_mine();

    void start(int player, int n_inputs);
    void stop(int player, const vector<int>& targets);
};

#endif /* PROCESSOR_INPUT_H_ */
