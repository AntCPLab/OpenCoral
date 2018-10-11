/*
 * Input.h
 *
 */

#ifndef PROCESSOR_INPUT_H_
#define PROCESSOR_INPUT_H_

#include <vector>
using namespace std;

#include "Math/Share.h"
#include "Auth/MAC_Check.h"
#include "Processor/Buffer.h"
#include "Tools/time-func.h"

class Processor;

template<class T>
class InputBase
{
protected:
    Buffer<T,T> buffer;
    Timer timer;

public:
    int values_input;

    InputBase(Processor& proc);
    ~InputBase();
};

template<class T>
class Input : public InputBase<T>
{
    Processor& proc;
    MAC_Check<T>& MC;
    vector< vector< Share<T> > > shares;

    void adjust_mac(Share<T>& share, T& value);

public:
    Input(Processor& proc, MAC_Check<T>& mc);

    void start(int player, int n_inputs);
    void stop(int player, vector<int> targets);

};

#endif /* PROCESSOR_INPUT_H_ */
