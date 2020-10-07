/*
 * OutputRedirection.h
 *
 */

#ifndef TOOLS_SWITCHABLEOUTPUT_H_
#define TOOLS_SWITCHABLEOUTPUT_H_

#include <iostream>
#include <fstream>
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

    void redirect_to_file(ofstream& out_file)
    {
        if (!on)
            this->on = true;
        cout.rdbuf(out_file.rdbuf());
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
