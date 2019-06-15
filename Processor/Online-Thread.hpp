
#include "Processor/Program.h"
#include "Processor/Online-Thread.h"
#include "Tools/time-func.h"
#include "Processor/Data_Files.h"
#include "Processor/Machine.h"
#include "Processor/Processor.h"
#include "Networking/CryptoPlayer.h"

#include "Processor/Processor.hpp"
#include "Processor/Input.hpp"

#include <iostream>
#include <fstream>
#include <pthread.h>
using namespace std;


template<class sint, class sgf2n>
void* Sub_Main_Func(void* ptr)
{
  bigint::init_thread();

  thread_info<sint, sgf2n> *tinfo=(thread_info<sint, sgf2n> *) ptr;
  Machine<sint, sgf2n>& machine=*(tinfo->machine);
  vector<pthread_mutex_t>& t_mutex      = machine.t_mutex;
  vector<pthread_cond_t>& client_ready  = machine.client_ready;
  vector<pthread_cond_t>& server_ready  = machine.server_ready;
  vector<Program>& progs                = machine.progs;

  int num=tinfo->thread_num;
#ifdef DEBUG_THREADS
  fprintf(stderr, "\tI am in thread %d\n",num);
#endif
  Player* player;
  if (machine.use_encryption)
    {
#ifdef VERBOSE
      cerr << "Using encrypted single-threaded communication" << endl;
#endif
      player = new CryptoPlayer(*(tinfo->Nms), num << 16);
    }
  else if (!machine.receive_threads or machine.direct or machine.parallel)
    {
#ifdef VERBOSE
      cerr << "Using single-threaded receiving" << endl;
#endif
      player = new PlainPlayer(*(tinfo->Nms), num << 16);
    }
  else
    {
      cerr << "Using player-specific threads for receiving" << endl;
      player = new ThreadPlayer(*(tinfo->Nms), num << 16);
    }
  Player& P = *player;
#ifdef DEBUG_THREADS
  fprintf(stderr, "\tSet up player in thread %d\n",num);
#endif

  typename sgf2n::MAC_Check* MC2;
  typename sint::MAC_Check*  MCp;

  // Use MAC_Check instead for more than 10000 openings at once
  if (machine.direct)
    {
      cerr << "Using direct communication. If computation stalls, use -m when compiling." << endl;
      MC2 = new typename sgf2n::Direct_MC(*(tinfo->alpha2i),*(tinfo->Nms), num);
      MCp = new typename sint::Direct_MC(*(tinfo->alphapi),*(tinfo->Nms), num);
    }
  else if (machine.parallel)
    {
      cerr << "Using indirect communication with background threads." << endl;
      //MC2 = new Parallel_MAC_Check<gf2n>(*(tinfo->alpha2i),*(tinfo->Nms), num, machine.opening_sum, machine.max_broadcast);
      //MCp = new Parallel_MAC_Check<gfp>(*(tinfo->alphapi),*(tinfo->Nms), num, machine.opening_sum, machine.max_broadcast);
      throw not_implemented();
    }
  else
    {
#ifdef VERBOSE
      cerr << "Using indirect communication." << endl;
#endif
      MC2 = new typename sgf2n::MAC_Check(*(tinfo->alpha2i), machine.opening_sum, machine.max_broadcast);
      MCp = new typename sint::MAC_Check(*(tinfo->alphapi), machine.opening_sum, machine.max_broadcast);
    }

  // Allocate memory for first program before starting the clock
  auto processor = new Processor<sint, sgf2n>(tinfo->thread_num,P,*MC2,*MCp,machine,progs[0]);
  auto& Proc = *processor;

  bool flag=true;
  int program=-3; 
  // int exec=0;

  // synchronize
#ifdef DEBUG_THREADS
  cerr << "Locking for sync of thread " << num << endl;
#endif
  pthread_mutex_lock(&t_mutex[num]);
  tinfo->ready=true;
  pthread_cond_signal(&client_ready[num]);
  pthread_mutex_unlock(&t_mutex[num]);

  DataPositions actual_usage(P.num_players());
  Timer thread_timer(CLOCK_THREAD_CPUTIME_ID), wait_timer;
  thread_timer.start();

  while (flag)
    { // Wait until I have a program to run
      wait_timer.start();
      pthread_mutex_lock(&t_mutex[num]);
        if ((tinfo->prognum)==-2)
	  { pthread_cond_wait(&server_ready[num],&t_mutex[num]); }
      program=(tinfo->prognum);
      (tinfo->prognum)=-2;
      pthread_mutex_unlock(&t_mutex[num]);
      wait_timer.stop();
      //printf("\tRunning program %d\n",program);

      if (program==-1)
        { flag=false;
#ifdef DEBUG_THREADS
          fprintf(stderr, "\tThread %d terminating\n",num);
#endif
        }
      else
        { // RUN PROGRAM
          //printf("\tClient %d about to run %d in execution %d\n",num,program,exec);
          Proc.reset(progs[program],tinfo->arg);

          // Bits, Triples, Squares, and Inverses skipping
          Proc.DataF.seekg(tinfo->pos);
          // reset for actual usage
          Proc.DataF.reset_usage();
             
          //printf("\tExecuting program");
          // Execute the program
          progs[program].execute(Proc);

          actual_usage.increase(Proc.DataF.get_usage());

         if (progs[program].usage_unknown())
           { // communicate file positions to main thread
             tinfo->pos.increase(Proc.DataF.get_usage());
           }

          //double elapsed = timeval_diff(&startv, &endv);
          //printf("Thread time = %f seconds\n",elapsed/1000000);
          //printf("\texec = %d\n",exec); exec++;
          //printf("\tMC2.number = %d\n",MC2.number());
          //printf("\tMCp.number = %d\n",MCp.number());

          // MACCheck
          MC2->Check(P);
          MCp->Check(P);
          //printf("\tMAC checked\n");
          P.Check_Broadcast();
          //printf("\tBroadcast checked\n");

         // printf("\tSignalling I have finished\n");
          wait_timer.start();
         pthread_mutex_lock(&t_mutex[num]);
            (tinfo->finished)=true;
            pthread_cond_signal(&client_ready[num]);
	 pthread_mutex_unlock(&t_mutex[num]);
	 wait_timer.stop();
       }  
    }

  // destruct protocol before last MAC check and data statistics
  size_t prep_sent = Proc.DataF.data_sent();
  delete processor;

  // MACCheck
  MC2->Check(P);
  MCp->Check(P);
  
  //cout << num << " : Checking broadcast" << endl;
  P.Check_Broadcast();
  //cout << num << " : Broadcast checked "<< endl;

  wait_timer.start();
  pthread_mutex_lock(&t_mutex[num]);
  if (!tinfo->ready)
    pthread_cond_wait(&server_ready[num], &t_mutex[num]);
  pthread_mutex_unlock(&t_mutex[num]);
  wait_timer.stop();

#ifdef VERBOSE
  cerr << num << " : MAC Checking" << endl;
  cerr << "\tMC2.number=" << MC2->number() << endl;
  cerr << "\tMCp.number=" << MCp->number() << endl;

  cerr << "Thread " << num << " timer: " << thread_timer.elapsed() << endl;
  cerr << "Thread " << num << " wait timer: " << wait_timer.elapsed() << endl;
#endif

  machine.data_sent += P.sent + prep_sent;
  tinfo->pos = actual_usage;

  delete MC2;
  delete MCp;
  delete player;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  OPENSSL_thread_stop();
#endif
  return NULL;
}


template<class sint, class sgf2n>
void* thread_info<sint, sgf2n>::Main_Func(void* ptr)
{
#ifndef INSECURE
  try
#endif
  {
      Sub_Main_Func<sint, sgf2n>(ptr);
  }
#ifndef INSECURE
  catch (...)
  {
      thread_info<sint, sgf2n>* ti = (thread_info<sint, sgf2n>*)ptr;
      ti->purge_preprocessing(*ti->machine);
      throw;
  }
#endif
  return 0;
}


template<class sint, class sgf2n>
void thread_info<sint, sgf2n>::purge_preprocessing(Machine<sint, sgf2n>& machine)
{
  cerr << "Purging preprocessed data because something is wrong" << endl;
  try
  {
      Data_Files<sint, sgf2n> df(machine);
      df.purge();
  }
  catch(...)
  {
      cerr << "Purging failed. This might be because preprocessed data is incomplete." << endl
          << "SECURITY FAILURE; YOU ARE ON YOUR OWN NOW!" << endl;
  }
}
