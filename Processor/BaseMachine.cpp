/*
 * BaseMachine.cpp
 *
 */

#include "BaseMachine.h"

#include <iostream>
using namespace std;

BaseMachine* BaseMachine::singleton = 0;

BaseMachine& BaseMachine::s()
{
  if (singleton)
    return *singleton;
  else
    throw runtime_error("no singleton");
}

BaseMachine::BaseMachine() : nthreads(0)
{
  if (not singleton)
    singleton = this;
}

void BaseMachine::load_schedule(string progname)
{
  this->progname = progname;
  string fname = "Programs/Schedules/" + progname + ".sch";
  cerr << "Opening file " << fname << endl;
  inpf.open(fname);
  if (inpf.fail()) { throw file_error("Missing '" + fname + "'. Did you compile '" + progname + "'?"); }

  int nprogs;
  inpf >> nthreads;
  inpf >> nprogs;

  cerr << "Number of threads I will run in parallel = " << nthreads << endl;
  cerr << "Number of program sequences I need to load = " << nprogs << endl;

  // Load in the programs
  string threadname;
  for (int i=0; i<nprogs; i++)
    { inpf >> threadname;
      string filename = "Programs/Bytecode/" + threadname + ".bc";
      cerr << "Loading program " << i << " from " << filename << endl;
      load_program(threadname, filename);
    }
}

void BaseMachine::print_compiler()
{

  char compiler[1000];
  inpf.get();
  inpf.getline(compiler, 1000);
  if (compiler[0] != 0)
    cerr << "Compiler: " << compiler << endl;
  inpf.close();
}

void BaseMachine::load_program(string threadname, string filename)
{
  (void)threadname;
  (void)filename;
  throw not_implemented();
}

void BaseMachine::time()
{
  cout << "Elapsed time: " << timer[0].elapsed() << endl;
}

void BaseMachine::start(int n)
{
  cout << "Starting timer " << n << " at " << timer[n].elapsed()
    << " after " << timer[n].idle() << endl;
  timer[n].start();
}

void BaseMachine::stop(int n)
{
  timer[n].stop();
  cout << "Stopped timer " << n << " at " << timer[n].elapsed() << endl;
}

void BaseMachine::print_timers()
{
  cerr << "Time = " << timer[0].elapsed() << " seconds " << endl;
  timer.erase(0);
  for (map<int,Timer>::iterator it = timer.begin(); it != timer.end(); it++)
    cerr << "Time" << it->first << " = " << it->second.elapsed() << " seconds " << endl;
}
