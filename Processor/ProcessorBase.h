/*
 * ProcessorBase.h
 *
 */

#ifndef PROCESSOR_PROCESSORBASE_H_
#define PROCESSOR_PROCESSORBASE_H_

#include <stack>
#include <string>
#include <fstream>
using namespace std;

class ProcessorBase
{
  // Stack
  stack<long> stacki;

  ifstream input_file;
  string input_filename;

protected:
  // Optional argument to tape
  int arg;

public:
  void pushi(long x) { stacki.push(x); }
  void popi(long& x) { x = stacki.top(); stacki.pop(); }

  int get_arg() const
    {
      return arg;
    }

  void set_arg(int new_arg)
    {
      arg=new_arg;
    }

  void open_input_file(const string& name);
  void open_input_file(int my_num, int thread_num);

  long long get_input(bool interactive);
  long long get_input(istream& is, const string& input_filename);
};

#endif /* PROCESSOR_PROCESSORBASE_H_ */
