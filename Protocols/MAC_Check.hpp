#ifndef PROTOCOLS_MAC_CHECK_HPP_
#define PROTOCOLS_MAC_CHECK_HPP_

#include "Protocols/MAC_Check.h"
#include "Tools/Subroutines.h"
#include "Exceptions/Exceptions.h"

#include "Tools/random.h"
#include "Tools/time-func.h"
#include "Tools/int.h"
#include "Tools/benchmarking.h"
#include "Tools/Bundle.h"

#include <algorithm>

#include "Protocols/MAC_Check_Base.hpp"

template<class T>
const char* TreeSum<T>::mc_timer_names[] = {
        "sending",
        "receiving and adding",
        "broadcasting",
        "receiving summed values",
        "random seed",
        "commit and open",
        "wait for summer thread",
        "receiving",
        "summing",
        "waiting for select()"
};

template<class U>
MAC_Check_<U>::MAC_Check_(const typename T::Scalar& ai, int opening_sum,
    int max_broadcast, int send_player) :
    TreeSum<T>(opening_sum, max_broadcast, send_player)
{
  popen_cnt=0;
  this->alphai=ai;
  vals.reserve(2 * POPEN_MAX);
  macs.reserve(2 * POPEN_MAX);
}

template<class T>
MAC_Check_<T>::~MAC_Check_()
{
}

template<class U>
void MAC_Check_<U>::PrepareSending(vector<T>& values, const vector<U>& S)
{
  values.resize(S.size());
  for (unsigned int i=0; i<S.size(); i++)
    { values[i]=S[i].get_share(); }
}

template<class U>
void MAC_Check_<U>::POpen_Begin(vector<T>& values,const vector<U>& S,const Player& P)
{
  AddToMacs(S);

  PrepareSending(values, S);

  this->start(values, P);

  this->values_opened += S.size();
}

template<class U>
void MAC_Check_<U>::POpen_End(vector<T>& values,const vector<U>& S,const Player& P)
{
  S.size();

  this->finish(values, P);

  popen_cnt += values.size();
  CheckIfNeeded(P);

  /* not compatible with continuous communication
  send_player++;
  if (send_player==P.num_players())
    { send_player=0; }
  */
}

template<class U>
void MAC_Check_<U>::AddToMacs(const vector<U>& shares)
{
  for (unsigned int i = 0; i < shares.size(); i++)
    macs.push_back(shares[i].get_mac());
#ifdef DEBUG_MAC
  if (shares.size())
    cout << "adding macs " << shares.back() << " / " <<
        shares.back().get_mac() << " / " << macs.back() << endl;
#endif
}


template<class U>
void MAC_Check_<U>::AddToValues(vector<T>& values)
{
  vals.insert(vals.end(), values.begin(), values.end());
}


template<class U>
void MAC_Check_<U>::GetValues(vector<T>& values)
{
  int size = values.size();
  if (popen_cnt + size > int(vals.size()))
    {
      stringstream ss;
      ss << "wanted " << values.size() << " values from " << popen_cnt << ", only " << vals.size() << " in store";
      throw out_of_range(ss.str());
    }
  values.clear();
  typename vector<T>::iterator first = vals.begin() + popen_cnt;
  values.insert(values.end(), first, first + size);
}


template<class T>
void MAC_Check_<T>::CheckIfNeeded(const Player& P)
{
  if (WaitingForCheck() >= POPEN_MAX)
    Check(P);
}


template <class U>
void MAC_Check_<U>::AddToCheck(const U& share, const T& value, const Player& P)
{
  macs.push_back(share.get_mac());
  vals.push_back(value);
  popen_cnt++;
  CheckIfNeeded(P);
}



template<class U>
void MAC_Check_<U>::Check(const Player& P)
{
  if (WaitingForCheck() == 0)
    return;

  //cerr << "In MAC Check : " << popen_cnt << endl;

  if (popen_cnt < 10)
    {
      vector<T> deltas;
      Bundle<octetStream> bundle(P);
      for (int i = 0; i < popen_cnt; i++)
        {
          deltas.push_back(vals[i] * this->alphai - macs[i]);
          deltas.back().pack(bundle.mine);
        }
      this->timers[COMMIT].start();
      Commit_And_Open_(bundle, P);
      this->timers[COMMIT].stop();
      for (auto& delta : deltas)
        {
          for (auto& os : bundle)
            if (&os != &bundle.mine)
              delta += os.get<T>();
          if (not delta.is_zero())
            throw mac_fail();
        }
    }
  else
    {
      octet seed[SEED_SIZE];
      this->timers[SEED].start();
      Create_Random_Seed(seed,P,SEED_SIZE);
      this->timers[SEED].stop();
      PRNG G;
      G.SetSeed(seed);

      U sj;
      T a,gami,temp;
      typename T::Scalar h;
      vector<T> tau(P.num_players());
      a.assign_zero();
      gami.assign_zero();
      for (int i=0; i<popen_cnt; i++)
        {
          h.almost_randomize(G);
          temp.mul(h,vals[i]);
          a.add(a,temp);

          temp.mul(h,macs[i]);
          gami.add(gami,temp);
        }

      temp.mul(this->alphai,a);
      tau[P.my_num()].sub(gami,temp);

      //cerr << "\tCommit and Open" << endl;
      this->timers[COMMIT].start();
      Commit_And_Open(tau,P);
      this->timers[COMMIT].stop();

      //cerr << "\tFinal Check" << endl;

      T t;
      t.assign_zero();
      for (int i=0; i<P.num_players(); i++)
        { t.add(t,tau[i]); }
      if (!t.is_zero()) { throw mac_fail(); }
    }

  vals.erase(vals.begin(), vals.begin() + popen_cnt);
  macs.erase(macs.begin(), macs.begin() + popen_cnt);

  popen_cnt=0;
}

template<class T>
int mc_base_id(int function_id, int thread_num)
{
  return (function_id << 28) + ((T::field_type() + 1) << 24) + (thread_num << 16);
}

template<class T, class U, class V, class W>
MAC_Check_Z2k<T, U, V, W>::MAC_Check_Z2k(const T& ai, int opening_sum, int max_broadcast, int send_player) :
    MAC_Check_<W>(ai, opening_sum, max_broadcast, send_player), prep(0)
{
}

template<class T, class U, class V, class W>
MAC_Check_Z2k<T, U, V, W>::MAC_Check_Z2k(const T& ai, Names& Nms,
        int thread_num) : MAC_Check_Z2k(ai)
{
    (void) Nms, (void) thread_num;
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::AddToCheck(const W& share, const T& value, const Player& P)
{
  shares.push_back(share.get_share());
  MAC_Check_<W>::AddToCheck(share, value, P);
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::AddToMacs(const vector<W >& shares)
{
  for (auto& share : shares)
    this->shares.push_back(share.get_share());
  MAC_Check_<W>::AddToMacs(shares);
#ifdef DEBUG_MAC
  cout << "add share " << shares.back() << " / " << this->shares.back() << endl;
#endif
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::PrepareSending(vector<T>& values,
        const vector<W >& S)
{
  values.clear();
  values.reserve(S.size());
  for (auto& share : S)
    values.push_back(V(share.get_share()));
}

template<class T, class U, class V, class W>
W MAC_Check_Z2k<T, U, V, W>::get_random_element() {
  if (random_elements.size() > 0)
    {
      W res = random_elements.back();
      random_elements.pop_back();
      return res;
    }
  else
    {
      if (prep)
        return prep->get_random();
      else
      {
        insecure("random dummy");
        return {};
      }
    }
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::set_random_element(const W& random_element) {
  random_elements.push_back(random_element);
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::set_prep(MascotPrep<W>& prep)
{
  this->prep = &prep;
}

template<class T, class U, class V, class W>
void MAC_Check_Z2k<T, U, V, W>::Check(const Player& P)
{
  if (this->WaitingForCheck() == 0)
    return;

#ifdef DEBUG_MAC
  cout << "Checking " << shares[0] << " " << this->vals[0] << " " << this->macs[0] << endl;
#endif

  int k = V::N_BITS;
  octet seed[SEED_SIZE];
  Create_Random_Seed(seed,P,SEED_SIZE);
  PRNG G;
  G.SetSeed(seed);

  T y, mj;
  y.assign_zero();
  mj.assign_zero();
  vector<U> chi;
  for (int i = 0; i < this->popen_cnt; ++i)
  {
    U temp_chi;
    temp_chi.randomize(G);
    T xi = this->vals[i];
    y += xi * temp_chi;
    T mji = this->macs[i];
    mj += temp_chi * mji;
    chi.push_back(temp_chi);
  }

  W r = get_random_element();
  T lj = r.get_mac();
  U pj;
  pj.assign_zero();
  for (int i = 0; i < this->popen_cnt; ++i)
  {
    T xji = shares[i];
    V xbarji = xji;
    U pji = U((xji - xbarji) >> k);
    pj += chi[i] * pji;
  }
  pj += U(r.get_share());

  U pbar(pj);
  vector<octetStream> pj_stream(P.num_players());
  pj.pack(pj_stream[P.my_num()]);
  P.Broadcast_Receive(pj_stream, true);
  for (int j=0; j<P.num_players(); j++) {
    if (j!=P.my_num()) {
      pbar += pj_stream[j].consume(U::size());
    }
  }

  T zj = mj - (this->alphai * y) - (((this->alphai * pbar)) << k) + (lj << k);
  vector<T> zjs(P.num_players());
  zjs[P.my_num()] = zj;
  Commit_And_Open(zjs, P);

  T zj_sum;
  zj_sum.assign_zero();
  for (int i = 0; i < P.num_players(); ++i)
    zj_sum += zjs[i];

  this->vals.erase(this->vals.begin(), this->vals.begin() + this->popen_cnt);
  this->macs.erase(this->macs.begin(), this->macs.begin() + this->popen_cnt);
  this->shares.erase(this->shares.begin(), this->shares.begin() + this->popen_cnt);
  this->popen_cnt=0;
  if (!zj_sum.is_zero()) { throw mac_fail(); }
}

template<class T>
Separate_MAC_Check<T>::Separate_MAC_Check(const typename T::mac_key_type& ai, Names& Nms,
    int thread_num, int opening_sum, int max_broadcast, int send_player) :
    MAC_Check_<T>(ai, opening_sum, max_broadcast, send_player),
    check_player(Nms, mc_base_id<T>(1, thread_num))
{
}

template<class T>
void Separate_MAC_Check<T>::Check(const Player& P)
{
  P.my_num();
  MAC_Check_<T>::Check(check_player);
}


template<class T>
void* run_summer_thread(void* summer)
{
  ((Summer<T>*) summer)->run();
  return 0;
}

template <class T>
Parallel_MAC_Check<T>::Parallel_MAC_Check(const T& ai, Names& Nms,
    int thread_num, int opening_sum, int max_broadcast, int base_player) :
    Separate_MAC_Check<T>(ai, Nms, thread_num, opening_sum, max_broadcast, base_player),
    send_player(Nms, mc_base_id<T>(2, thread_num)),
    send_base_player(base_player)
{
  int sum_players = Nms.num_players();
  Player* summer_send_player = &send_player;
  for (int i = 0; ; i++)
    {
      int last_sum_players = sum_players;
      sum_players = (sum_players - 2 + opening_sum) / opening_sum;
      int next_sum_players = (sum_players - 2 + opening_sum) / opening_sum;
      if (sum_players == 0)
        break;
      Player* summer_receive_player = summer_send_player;
      summer_send_player = new PlainPlayer(Nms, mc_base_id<T>(3, thread_num));
      summers.push_back(new Summer<T>(sum_players, last_sum_players, next_sum_players,
              summer_send_player, summer_receive_player, *this));
      pthread_create(&(summers[i]->thread), 0, run_summer_thread<T>, summers[i]);
    }
  receive_player = summer_send_player;
}

template<class T>
Parallel_MAC_Check<T>::~Parallel_MAC_Check()
{
  for (unsigned int i = 0; i < summers.size(); i++)
    {
      summers[i]->input_queue.stop();
      pthread_join(summers[i]->thread, 0);
      delete summers[i];
    }
}

template<class T>
void Parallel_MAC_Check<T>::POpen_Begin(vector<T>& values,
        const vector<Share<T> >& S, const Player& P)
{
  values.size();
  this->AddToMacs(S);

  int my_relative_num = positive_modulo(P.my_num() - send_base_player, P.num_players());
  int sum_players = (P.num_players() - 2 + this->opening_sum) / this->opening_sum;
  int receiver = positive_modulo(send_base_player + my_relative_num % sum_players, P.num_players());

  // use queue rather sending to myself
  if (receiver == P.my_num())
    {
      for (unsigned int i = 0; i < S.size(); i++)
        values[i] = S[i].get_share();
      summers.front()->share_queue.push(values);
    }
  else
    {
      this->os.reset_write_head();
      for (unsigned int i=0; i<S.size(); i++)
          S[i].get_share().pack(this->os);
      this->timers[SEND].start();
      send_player.send_to(receiver,this->os,true);
      this->timers[SEND].stop();
    }

  for (unsigned int i = 0; i < summers.size(); i++)
      summers[i]->input_queue.push(S.size());

  this->values_opened += S.size();
  send_base_player = (send_base_player + 1) % send_player.num_players();
}

template<class T>
void Parallel_MAC_Check<T>::POpen_End(vector<T>& values,
        const vector<Share<T> >& S, const Player& P)
{
  int last_size = 0;
  this->timers[WAIT_SUMMER].start();
  summers.back()->output_queue.pop(last_size);
  this->timers[WAIT_SUMMER].stop();
  if (int(values.size()) != last_size)
    {
      stringstream ss;
      ss << "stopopen wants " << values.size() << " values, but I have " << last_size << endl;
      throw Processor_Error(ss.str().c_str());
    }

  if (this->base_player == P.my_num())
    {
      value_queue.pop(values);
      if (int(values.size()) != last_size)
        throw Processor_Error("wrong number of local values");
      else
        this->AddToValues(values);
    }
  this->MAC_Check<T>::POpen_End(values, S, *receive_player);
  this->base_player = (this->base_player + 1) % send_player.num_players();
}



template<class T>
Direct_MAC_Check<T>::Direct_MAC_Check(const typename T::mac_key_type& ai, Names& Nms, int num) : Separate_MAC_Check<T>(ai, Nms, num) {
  open_counter = 0;
}


template<class T>
Direct_MAC_Check<T>::~Direct_MAC_Check() {
  cerr << T::type_string() << " open counter: " << open_counter << endl;
}


template<class T>
void Direct_MAC_Check<T>::POpen_Begin(vector<open_type>& values,const vector<T>& S,const Player& P)
{
  values.resize(S.size());
  this->os.reset_write_head();
  for (unsigned int i=0; i<S.size(); i++)
    S[i].get_share().pack(this->os);
  this->timers[SEND].start();
  P.send_all(this->os,true);
  this->timers[SEND].stop();

  this->AddToMacs(S);
  for (unsigned int i=0; i<S.size(); i++)
    this->vals.push_back(S[i].get_share());
}

template<class T, int t>
void direct_add_openings(vector<T>& values, const Player& P, vector<octetStream>& os)
{
  for (unsigned int i=0; i<values.size(); i++)
    for (int j=0; j<P.num_players(); j++)
      if (j!=P.my_num())
	values[i].template add<t>(os[j]);
}

template<class T>
void Direct_MAC_Check<T>::POpen_End(vector<open_type>& values,const vector<T>& S,const Player& P)
{
  S.size();
  oss.resize(P.num_players());
  this->GetValues(values);

  this->timers[RECV].start();

  for (int j=0; j<P.num_players(); j++)
    if (j!=P.my_num())
      P.receive_player(j,oss[j],true);

  this->timers[RECV].stop();
  open_counter++;

  if (open_type::t() == 2)
    direct_add_openings<open_type,2>(values, P, oss);
  else
    direct_add_openings<open_type,0>(values, P, oss);

  for (unsigned int i = 0; i < values.size(); i++)
    this->vals[this->popen_cnt+i] = values[i];

  this->popen_cnt += values.size();
  this->CheckIfNeeded(P);
}

template<class T>
Passing_MAC_Check<T>::Passing_MAC_Check(const T& ai, Names& Nms, int num) :
  Separate_MAC_Check<Share<T>>(ai, Nms, num)
{
}

template<class T, int t>
void passing_add_openings(vector<T>& values, octetStream& os)
{
  octetStream new_os;
  for (unsigned int i=0; i<values.size(); i++)
    {
      T tmp;
      tmp.unpack(os);
      (tmp + values[i]).pack(new_os);
    }
  os = new_os;
}

template<class T>
void Passing_MAC_Check<T>::POpen_Begin(vector<T>& values,const vector<Share<T> >& S,const Player& P)
{
  values.resize(S.size());
  this->os.reset_write_head();
  for (unsigned int i=0; i<S.size(); i++)
    {
      S[i].get_share().pack(this->os);
      values[i] = S[i].get_share();
    }
  this->AddToMacs(S);

  for (int i = 0; i < P.num_players() - 1; i++)
    {
      P.pass_around(this->os);
      if (T::t() == 2)
        passing_add_openings<T,2>(values, this->os);
      else
        passing_add_openings<T,0>(values, this->os);
    }

  for (unsigned int i = 0; i < values.size(); i++)
    {
      T tmp;
      tmp.unpack(this->os);
      this->vals.push_back(tmp);
    }
}

template<class T>
void Passing_MAC_Check<T>::POpen_End(vector<T>& values,const vector<Share<T> >& S,const Player& P)
{
  (void)S;
  this->GetValues(values);
  this->popen_cnt += values.size();
  this->CheckIfNeeded(P);
}

#endif
