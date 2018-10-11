/*
 * OutputRedirection.h
 *
 */

#ifndef TOOLS_SWITCHABLEOUTPUT_H_
#define TOOLS_SWITCHABLEOUTPUT_H_

#include <iostream>
using namespace std;

class SwitchableOutput
{
    bool on;

public:
    SwitchableOutput(bool on = true)
    {
        activate(on);
    }

    void activate(bool on)
    {
        this->on = on;
    }

    template<class T>
    SwitchableOutput& operator<<(const T& value)
    {
        if (on)
            cout << value;
        return *this;

        cout << flush;
    }

    SwitchableOutput& operator<<(ostream& (*__pf)(ostream&))
    {
        if (on)
            cout << __pf;
        return *this;
    }
};

#endif /* TOOLS_SWITCHABLEOUTPUT_H_ */
