
// [zico] need to update
#define NO_SECURITY_CHECK
#define VERBOSE_DEBUG_PRINT

#include "Machines/Coral.hpp"
#include "Protocols/ProtocolSet.h"
#include "Tools/performance.h"

typedef CoralLowGearShare<gfp_<0, 2>> sint;
typedef Share<gf2n_short> sgf2n;

class my_thread_info : public thread_info<sint, sgf2n> {
public:
ThreadQueue* queue;
};

void* thread_run(void* ptr) {
auto tinfo = (my_thread_info*)(ptr);

  int num=tinfo->thread_num;
  BaseMachine::s().thread_num = num;

  auto& queues = tinfo->queue;
  queues->next();
  ThreadQueue::thread_queue = queues;

  Player* player;
  string id = "thread" + to_string(num);
    player = new PlainPlayer(*(tinfo->Nms), id);
  Player& P = *player;

  typename sint::MAC_Check*  MCp;

      MCp = new typename sint::MAC_Check(*(tinfo->alphapi), 0, 0);

  // Allocate memory for first program before starting the clock
  
  sint::Protocol::setup(P);
  sint::bit_type::Protocol::setup(P);
  sgf2n::Protocol::setup(P);

DataPositions usage(P.num_players());
    sint::LivePrep preprocessing(0, usage);
    sint::bit_type::LivePrep bit_prep(usage);
  SubProcessor<sint>* subproc = new SubProcessor<sint>(*MCp, preprocessing, P);
  GC::ShareThread<typename sint::bit_type> share_thread(bit_prep, P, {});

queues->finished({});
    while (true)
    { // Wait until I have a program to run
      ThreadJob job = queues->next();

      if (job.type == EDABIT_SACRIFICE_JOB)
        {
            sint::LivePrep::edabit_sacrifice_buckets(
                *(vector<edabitpack<sint>>*) job.output, job.length, job.prognum,
                job.arg, *subproc,
                job.begin, job.end, job.supply);
          queues->finished(job);
        }
        else if (job.type == PERSONAL_QUINTUPLE_JOB)
        {
          auto &party = GC::ShareThread<typename sint::bit_type>::s();
          SubProcessor<sint::bit_type> bit_proc(party.MC->get_part_MC(),
              *subproc->personal_bit_preps.at(job.arg), P);
          subproc->personal_bit_preps.at(job.arg)->buffer_personal_quintuples(
              *(vector<array<sint::bit_type, 5>>*) job.output, job.begin, job.end);
          queues->finished(job);
        }
    }
delete subproc;
delete MCp;
delete player;

}

template<class T>
void test_buffer_edabits(int argc, const char** argv, bool strict, int prime_length = 0)
{
    OnlineOptions::singleton.batch_size = 10000;
    cout << "[zico] batch size: " << OnlineOptions::singleton.batch_size << endl;
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);

    PlainPlayer P(N);

    // protocol setup (domain, MAC key if needed etc)
    // Need to call this first so that a MAC key is generated and written to a directory
    // And Spdz2k A and B sharings will read the same key
    // read_generate_write_mac_key<T>(P);
    MixedProtocolSetup<T> setup(P, prime_length, get_prep_sub_dir<T>(P.num_players()), true);

    // set of protocols (input, multiplication, output)
    MixedProtocolSet<T> set(P, setup);
    auto& prep = set.preprocessing;
    typename T::mac_key_type alphapi = setup.get_mac_key();

    int nthreads = 64;
    ThreadQueues queues;
    queues.resize(nthreads);
     vector<pthread_t> threads;
     threads.resize(nthreads);
     vector<my_thread_info> tinfo;
     tinfo.resize(nthreads);

    BaseMachine machine;
    machine.queues = queues;

  for (int i = 0; i < nthreads; i++)
    {
      queues[i] = new ThreadQueue;
      // stand-in for initialization
      queues[i]->schedule({});
      tinfo[i].thread_num=i;
      tinfo[i].Nms=&N;
      tinfo[i].alphapi=&alphapi;
      tinfo[i].alpha2i={};
      tinfo[i].machine=0;
      tinfo[i].queue = queues[i];
      pthread_create(&threads[i],NULL,thread_run,&tinfo[i]);
    }
    // synchronize with clients before starting timer
  for (int i=0; i<nthreads; i++)
    {
      queues[i]->result();
    }
    

    auto start = perf_log((strict? "strict" : "loose") + string(" edabit"), P.total_comm().sent);
    for (int i = 0; i < 2; i++)
        prep.buffer_edabits(strict, 64, &queues);
    auto diff = perf_log((strict? "strict" : "loose") + string(" edabit"), P.total_comm().sent);
    cout << "[Time/1000 ops] " << diff.first.count() * 1.0 / 1e6 / prep.get_edabit_size(strict, 64) * 1000 << " ms" << endl;
    cout << "[Comm/1000 ops] " << diff.second * 1.0 / 1e6 / prep.get_edabit_size(strict, 64) * 1000 << " MB" << endl;

      // Wait until all clients have signed out
  for (int i=0; i<nthreads; i++)
    {
      queues[i]->schedule({});
      pthread_join(threads[i],NULL);
    }


    set.check();

      for (int i = 0; i < nthreads; i++)
    {
      delete queues[i];
    }
}


int main(int argc, const char** argv)
{
    cerr << "Usage: " << argv[0]
        << "<my number: 0/1/...> <total number of players> [protocol] [buffer_type] [strict|loose]"
        << endl;

    // bit length of prime
    const int prime_length = 128;

    // compute number of 64-bit words needed
    const int n_limbs = (prime_length + 63) / 64;

    string protocol = "coral";
    if (argc > 3)
        protocol = argv[3]; // coral, corallowgear, coralmascot, spdz2k, lowgear, mascot

    string type = "edabit";
    if (argc > 4)
        type = argv[4]; // edabit, dabit

    bool strict = false;
    if (argc > 5)
        strict = (string(argv[5]) == string("strict"));
    
    /* edabit */
    // if (protocol == "coral" && type == "edabit") {
    //     test_buffer_edabits<CoralShare<64, 64>>(argc, argv, strict);
    // }
    if (protocol == "corallowgear" && type == "edabit") {
        ez::ezOptionParser opt;
        CowGearOptions::singleton = CowGearOptions(opt, argc, argv, false);
        test_buffer_edabits<CoralLowGearShare<gfp_<0, n_limbs>>>(argc, argv, strict, prime_length);
    }

}
