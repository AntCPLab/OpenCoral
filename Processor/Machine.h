/*
 * Machine.h
 *
 */

#ifndef MACHINE_H_
#define MACHINE_H_

#include "Processor/Memory.h"
#include "Processor/Program.h"

#include "Processor/Online-Thread.h"
#include "Processor/Data_Files.h"
#include "Math/gfp.h"

#include "Tools/time-func.h"

#include <vector>
#include <map>
using namespace std;

class BaseMachine
{
protected:
    static BaseMachine* singleton;

    std::map<int,Timer> timer;

    ifstream inpf;

    void print_timers();

    virtual void load_program(string threadname, string filename);

public:
    string progname;
    int nthreads;

    static BaseMachine& s();

    BaseMachine();
    virtual ~BaseMachine() {}

    void load_schedule(string progname);
    void print_compiler();

    void time();
    void start(int n);
    void stop(int n);

    virtual void reqbl(int n) { (void)n; }
};

template<class sint>
class Machine : public BaseMachine
{
  /* The mutex's lock the C-threads and then only release
   * then we an MPC thread is ready to run on the C-thread.
   * Control is passed back to the main loop when the
   * MPC thread releases the mutex
   */

  vector<thread_info<sint>> tinfo;
  vector<pthread_t> threads;

  int my_number;
  Names& N;
  gfp  alphapi;
  gf2n alpha2i;

  // Keep record of used offline data
  DataPositions pos;

  int tn,numt;
  bool usage_unknown;

  void load_program(string threadname, string filename);

  public:

  vector<pthread_mutex_t> t_mutex;
  vector<pthread_cond_t>  client_ready;
  vector<pthread_cond_t>  server_ready;
  vector<Program>  progs;

  Memory<sgf2n> M2;
  Memory<sint> Mp;
  Memory<Integer> Mi;

  vector<Timer> join_timer;
  Timer finish_timer;
  
  string prep_dir_prefix;

  bool direct;
  int opening_sum;
  bool parallel;
  bool receive_threads;
  int max_broadcast;

  Machine(int my_number, Names& playerNames, string progname,
      string memtype, int lgp, int lg2, bool direct, int opening_sum, bool parallel,
      bool receive_threads, int max_broadcast);

  DataPositions run_tape(int thread_number, int tape_number, int arg, int line_number);
  void join_tape(int thread_number);
  void run();

  // Only for Player-Demo.cpp
  Machine(): N(*(new Names())) {}

  void reqbl(int n);
};

#endif /* MACHINE_H_ */
