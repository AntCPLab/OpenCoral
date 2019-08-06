#include "Machine.h"

#include "Memory.hpp"
#include "Online-Thread.hpp"

#include "Exceptions/Exceptions.h"

#include <sys/time.h>

#include "Math/Setup.h"
#include "Tools/mkpath.h"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <pthread.h>
using namespace std;

template<class sint, class sgf2n>
Machine<sint, sgf2n>::Machine(int my_number, Names& playerNames,
    string progname_str, string memtype, int lg2, bool direct,
    int opening_sum, bool parallel, bool receive_threads, int max_broadcast,
    bool use_encryption, bool live_prep, OnlineOptions opts)
  : my_number(my_number), N(playerNames), tn(0), numt(0), usage_unknown(false),
    direct(direct), opening_sum(opening_sum), parallel(parallel),
    receive_threads(receive_threads), max_broadcast(max_broadcast),
    use_encryption(use_encryption), live_prep(live_prep), opts(opts),
    data_sent(0)
{
  if (opening_sum < 2)
    this->opening_sum = N.num_players();
  if (max_broadcast < 2)
    this->max_broadcast = N.num_players();

  // Set up the fields
  prep_dir_prefix = get_prep_dir(N.num_players(), opts.lgp, lg2);
  char filename[2048];
  bool read_mac_keys = false;

  sgf2n::clear::init_field(lg2);

  try
    {
      sint::clear::read_setup(N.num_players(), opts.lgp, lg2);
      ::read_mac_keys(prep_dir_prefix, my_number, N.num_players(), alphapi, alpha2i);
      read_mac_keys = true;
    }
  catch (file_error& e)
    {
#ifdef VERBOSE
      cerr << "Field or MAC key setup failed, using defaults"
          << endl;
#endif
      sint::clear::init_default(opts.lgp);
      // make directory for outputs if necessary
      mkdir_p(PREP_DIR);
    }
  catch (end_of_file& e)
    {
      cerr << "End of file reading MAC key but maybe we don't need it" << endl;
    }

  auto P = new PlainPlayer(N, 0xF00);
  sint::LivePrep::basic_setup(*P);
  delete P;

  if (not read_mac_keys)
    {
#ifdef VERBOSE
      cerr << "Generating fresh MAC keys" << endl;
#endif
      SeededPRNG G;
      alphapi.randomize(G);
      alpha2i.randomize(G);

#ifdef DEBUG_MAC
      alpha2i = my_number;
      alphapi = my_number;
      alphapi = alphapi << 60;
#endif
    }

#ifdef DEBUG_MAC
  cerr << "MAC Key p = " << alphapi << endl;
  cerr << "MAC Key 2 = " << alpha2i << endl;
#endif

  // for OT-based preprocessing
  sint::clear::next::template init<typename sint::clear>(false);

  // Initialize the global memory
  if (memtype.compare("new")==0)
     {sprintf(filename, PREP_DIR "Player-Memory-P%d", my_number);
       ifstream memfile(filename);
       if (memfile.fail()) { throw file_error(filename); }
       M2.Load_Memory(memfile);
       Mp.Load_Memory(memfile);
       Mi.Load_Memory(memfile);
       memfile.close();
     }
  else if (memtype.compare("old")==0)
     {
       inpf.open(memory_filename(), ios::in | ios::binary);
       if (inpf.fail()) { throw file_error(); }
       inpf >> M2 >> Mp >> Mi;
       inpf.close();
     }
  else if (!(memtype.compare("empty")==0))
     { cerr << "Invalid memory argument" << endl;
       exit(1);
     }

  // Keep record of used offline data
  pos.set_num_players(N.num_players());

  load_schedule(progname_str);

#ifdef VERBOSE
  progs[0].print_offline_cost();
#endif

  if (live_prep and (sint::needs_ot or sgf2n::needs_ot))
  {
    Player* P;
    if (use_encryption)
      P = new CryptoPlayer(playerNames, 0xF000);
    else
      P = new PlainPlayer(playerNames, 0xF000);
    ot_setups.resize(nthreads);
    for (int i = 0; i < nthreads; i++)
      for (int j = 0; j < 3; j++)
        ot_setups.at(i).push_back({ *P, true });
    delete P;
  }

  /* Set up the threads */
  tinfo.resize(nthreads);
  threads.resize(nthreads);
  t_mutex.resize(nthreads);
  client_ready.resize(nthreads);
  server_ready.resize(nthreads);
  join_timer.resize(nthreads);

  for (int i=0; i<nthreads; i++)
    { pthread_mutex_init(&t_mutex[i],NULL);
      pthread_cond_init(&client_ready[i],NULL);
      pthread_cond_init(&server_ready[i],NULL);
      tinfo[i].thread_num=i;
      tinfo[i].Nms=&N;
      tinfo[i].alphapi=&alphapi;
      tinfo[i].alpha2i=&alpha2i;
      tinfo[i].prognum=-2;  // Dont do anything until we are ready
      tinfo[i].finished=true;
      tinfo[i].ready=false;
      tinfo[i].machine=this;
      // lock for synchronization
      pthread_mutex_lock(&t_mutex[i]);
      pthread_create(&threads[i],NULL,thread_info<sint, sgf2n>::Main_Func,&tinfo[i]);
    }

  // synchronize with clients before starting timer
  for (int i=0; i<nthreads; i++)
    {
      while (!tinfo[i].ready)
        {
#ifdef DEBUG_THREADS
          cerr << "Waiting for thread " << i << " to be ready" << endl;
#endif
          pthread_cond_wait(&client_ready[i],&t_mutex[i]);
        }
      pthread_mutex_unlock(&t_mutex[i]);
    }
}

template<class sint, class sgf2n>
void Machine<sint, sgf2n>::load_program(string threadname, string filename)
{
  ifstream pinp(filename);
  if (pinp.fail()) { throw file_error(filename); }
  progs.push_back(N.num_players());
  int i = progs.size() - 1;
  progs[i].parse(pinp);
  pinp.close();
  M2.minimum_size(GF2N, progs[i], threadname);
  Mp.minimum_size(MODP, progs[i], threadname);
  Mi.minimum_size(INT, progs[i], threadname);
}

template<class sint, class sgf2n>
DataPositions Machine<sint, sgf2n>::run_tape(int thread_number, int tape_number, int arg, int line_number)
{
  if (thread_number >= (int)tinfo.size())
    throw Processor_Error("invalid thread number: " + to_string(thread_number) + "/" + to_string(tinfo.size()));
  if (tape_number >= (int)progs.size())
    throw Processor_Error("invalid tape number: " + to_string(tape_number) + "/" + to_string(progs.size()));
  pthread_mutex_lock(&t_mutex[thread_number]);
  tinfo[thread_number].prognum=tape_number;
  tinfo[thread_number].arg=arg;
  tinfo[thread_number].pos=pos;
  tinfo[thread_number].finished=false;
  //printf("Send signal to run program %d in thread %d\n",tape_number,thread_number);
  pthread_cond_signal(&server_ready[thread_number]);
  pthread_mutex_unlock(&t_mutex[thread_number]);
  //printf("Running line %d\n",exec);
  if (progs[tape_number].usage_unknown())
    { // only one thread allowed
      if (numt>1)
        { cerr << "Line " << line_number << " has " <<
          numt << " threads but tape " << tape_number <<
          " has unknown offline data usage" << endl;
        throw invalid_program();
        }
      else if (line_number == -1)
        {
          cerr << "Internally called tape " << tape_number <<
              " has unknown offline data usage" << endl;
          throw invalid_program();
        }
      usage_unknown = true;
      return DataPositions(N.num_players());
    }
  else
    {
      // Bits, Triples, Squares, and Inverses skipping
      return progs[tape_number].get_offline_data_used();
    }
}

template<class sint, class sgf2n>
void Machine<sint, sgf2n>::join_tape(int i)
{
  join_timer[i].start();
  pthread_mutex_lock(&t_mutex[i]);
  //printf("Waiting for client to terminate\n");
  if ((tinfo[i].finished)==false)
    { pthread_cond_wait(&client_ready[i],&t_mutex[i]); }
  pthread_mutex_unlock(&t_mutex[i]);
  join_timer[i].stop();
}

template<class sint, class sgf2n>
void Machine<sint, sgf2n>::run()
{
  Timer proc_timer(CLOCK_PROCESS_CPUTIME_ID);
  proc_timer.start();
  timer[0].start();

  bool flag=true;
  usage_unknown=false;
  int exec=0;
  while (flag)
    { inpf >> numt;
      if (numt==0) 
        { flag=false; }
      else
        { for (int i=0; i<numt; i++)
            {
	        // Now load up data
                inpf >> tn;

                // Cope with passing an integer parameter to a tape
                int arg;
                if (inpf.get() == ':')
                  inpf >> arg;
                else
                  arg = 0;

                //cerr << "Run scheduled tape " << tn << " in thread " << i << endl;
                pos.increase(run_tape(i, tn, arg, exec));
            }
          // Make sure all terminate before we continue
          for (int i=0; i<numt; i++)
            { join_tape(i);
            }
         if (usage_unknown)
           { // synchronize files
             pos = tinfo[0].pos;
             usage_unknown = false;
           }
         //printf("Finished running line %d\n",exec);
         exec++;
      }
    }

  print_compiler();

  finish_timer.start();
  // Tell all C-threads to stop
  for (int i=0; i<nthreads; i++)
    { pthread_mutex_lock(&t_mutex[i]);
	//printf("Send kill signal to client\n");
        tinfo[i].prognum=-1;
        tinfo[i].ready = false;
        pthread_cond_signal(&server_ready[i]);
      pthread_mutex_unlock(&t_mutex[i]);
    }

  // reset to sum actual usage
  pos.reset();

#ifdef DEBUG_THREADS
  cerr << "Waiting for all clients to finish" << endl;
#endif
  // Wait until all clients have signed out
  for (int i=0; i<nthreads; i++)
    {
      pthread_mutex_lock(&t_mutex[i]);
      tinfo[i].ready = true;
      pthread_cond_signal(&server_ready[i]);
      pthread_mutex_unlock(&t_mutex[i]);
      pthread_join(threads[i],NULL);
      pthread_mutex_destroy(&t_mutex[i]);
      pthread_cond_destroy(&client_ready[i]);
      pthread_cond_destroy(&server_ready[i]);
      pos.increase(tinfo[i].pos);
    }
  finish_timer.stop();
  
#ifdef VERBOSE
  for (unsigned int i = 0; i < join_timer.size(); i++)
    cerr << "Join timer: " << i << " " << join_timer[i].elapsed() << endl;
  cerr << "Finish timer: " << finish_timer.elapsed() << endl;
  cerr << "Process timer: " << proc_timer.elapsed() << endl;
#endif

  print_timers();
  cerr << "Data sent = " << data_sent / 1e6 << " MB" << endl;

#ifdef VERBOSE
  if (opening_sum < N.num_players() && !direct)
    cerr << "Summed at most " << opening_sum << " shares at once with indirect communication" << endl;
  else
    cerr << "Summed all shares at once" << endl;

  if (max_broadcast < N.num_players() && !direct)
    cerr << "Send to at most " << max_broadcast << " parties at once" << endl;
  else
    cerr << "Full broadcast" << endl;
#endif

  // Reduce memory size to speed up
  unsigned max_size = 1 << 20;
  if (M2.size_s() > max_size)
    M2.resize_s(max_size);
  if (Mp.size_s() > max_size)
    Mp.resize_s(max_size);

  // Write out the memory to use next time
  ofstream outf(memory_filename(), ios::out | ios::binary);
  outf << M2 << Mp << Mi;
  outf.close();

#ifdef OLD_USAGE
  for (int dtype = 0; dtype < N_DTYPE; dtype++)
    {
      cerr << "Num " << DataPositions::dtype_names[dtype] << "\t=";
      for (int field_type = 0; field_type < N_DATA_FIELD_TYPE; field_type++)
        cerr << " " << pos.files[field_type][dtype];
      cerr << endl;
   }
  for (int field_type = 0; field_type < N_DATA_FIELD_TYPE; field_type++)
    {
      cerr << "Num " << DataPositions::field_names[field_type] << " Inputs\t=";
      for (int i = 0; i < N.num_players(); i++)
        cerr << " " << pos.inputs[i][field_type];
      cerr << endl;
    }
#endif

#ifdef VERBOSE
  cerr << "Actual cost of program:" << endl;
  pos.print_cost();
#endif

#ifndef INSECURE
  Data_Files<sint, sgf2n> df(*this);
  df.seekg(pos);
  df.prune();
#endif

  sint::LivePrep::teardown();
  sgf2n::LivePrep::teardown();

#ifdef VERBOSE
  cerr << "End of prog" << endl;
#endif
}

template<class sint, class sgf2n>
string Machine<sint, sgf2n>::memory_filename()
{
  return BaseMachine::memory_filename(sint::type_short(), my_number);
}

template<class sint, class sgf2n>
void Machine<sint, sgf2n>::reqbl(int n)
{
  sint::clear::reqbl(n);
}
