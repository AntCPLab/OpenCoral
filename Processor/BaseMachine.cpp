/*
 * BaseMachine.cpp
 *
 */

#include "BaseMachine.h"

#include <iostream>
#include <sodium.h>
using namespace std;

BaseMachine* BaseMachine::singleton = 0;
thread_local int BaseMachine::thread_num;

BaseMachine& BaseMachine::s()
{
  if (singleton)
    return *singleton;
  else
    throw runtime_error("no singleton");
}

BaseMachine::BaseMachine() : nthreads(0)
{
  if (sodium_init() == -1)
    throw runtime_error("couldn't initialize libsodium");
  if (not singleton)
    singleton = this;
}

void BaseMachine::load_schedule(string progname)
{
  this->progname = progname;
  string fname = "Programs/Schedules/" + progname + ".sch";
#ifdef DEBUG_FILES
  cerr << "Opening file " << fname << endl;
#endif
  inpf.open(fname);
  if (inpf.fail()) { throw file_error("Missing '" + fname + "'. Did you compile '" + progname + "'?"); }

  int nprogs;
  inpf >> nthreads;
  inpf >> nprogs;

  if (inpf.fail())
    throw file_error("Error reading " + fname);

#ifdef DEBUG_FILES
  cerr << "Number of threads I will run in parallel = " << nthreads << endl;
  cerr << "Number of program sequences I need to load = " << nprogs << endl;
#endif

  // Load in the programs
  string threadname;
  for (int i=0; i<nprogs; i++)
    { inpf >> threadname;
      string filename = "Programs/Bytecode/" + threadname + ".bc";
#ifdef DEBUG_FILES
      cerr << "Loading program " << i << " from " << filename << endl;
#endif
      load_program(threadname, filename);
    }
}

void BaseMachine::print_compiler()
{

  char compiler[1000];
  inpf.get();
  inpf.getline(compiler, 1000);
#ifdef VERBOSE
  if (compiler[0] != 0)
    cerr << "Compiler: " << compiler << endl;
#endif
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

string BaseMachine::memory_filename(string type_short, int my_number)
{
  return PREP_DIR "Memory-" + type_short + "-P" + to_string(my_number);
}
