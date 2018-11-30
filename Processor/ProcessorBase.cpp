/*
 * ProcessorBase.cpp
 *
 */

#include "ProcessorBase.h"
#include "Exceptions/Exceptions.h"

#include <iostream>

void ProcessorBase::open_input_file(const string& name)
{
    cerr << "opening " << name << endl;
    input_file.open(name);
    input_filename = name;
}

void ProcessorBase::open_input_file(int my_num, int thread_num)
{
    string input_file = "Player-Data/Input-P" + to_string(my_num) + "-" + to_string(thread_num);
    open_input_file(input_file);
}

long long ProcessorBase::get_input(bool interactive)
{
    if (interactive)
        return get_input(cin, "standard input");
    else
        return get_input(input_file, input_filename);
}

long long ProcessorBase::get_input(istream& input_file, const string& input_filename)
{
    long long res;
    input_file >> res;
    if (input_file.eof())
        throw IO_Error("not enough inputs in " + input_filename);
    if (input_file.fail())
        throw IO_Error("cannot read from " + input_filename);
    return res;
}
