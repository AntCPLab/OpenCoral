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
MAC_Check_<U>::MAC_Check_(const typename U::mac_key_type::Scalar& ai, int opening_sum,
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
  if (WaitingForCheck() > 0)
    {
      cerr << endl << "SECURITY BUG: insufficient checking" << endl;
      terminate();
    }
}

template<class U>
void MAC_Check_<U>::init_open(const Player&, int n)
{
  macs.reserve(macs.size() + n);
  this->secrets.clear();
  this->values.clear();
  this->secrets.reserve(n);
  this->values.reserve(n);
}

template<class U>
void MAC_Check_<U>::prepare_open(const U& secret)
{
  this->values.push_back(secret.get_share());
  macs.push_back(secret.get_mac());
}

template<class U>
void MAC_Check_<U>::exchange(const Player& P)
{
  this->run(this->values, P);

  this->values_opened += this->values.size();

  popen_cnt += this->values.size();
  CheckIfNeeded(P);

  /* not compatible with continuous communication
  send_player++;
  if (send_player==P.num_players())
    { send_player=0; }
  */
}


template<class U>
void MAC_Check_<U>::AddToValues(vector<T>& values)
{
  vals.insert(vals.end(), values.begin(), values.end());
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
      vector<typename U::mac_type> deltas;
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
              delta += os.get<typename U::mac_type>();
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
      typename U::mac_type a,gami,temp;
      typename U::mac_type::Scalar h;
      vector<typename U::mac_type> tau(P.num_players());
      a.assign_zero();
      gami.assign_zero();
      for (int i=0; i<popen_cnt; i++)
        {
          h.almost_randomize(G);
          temp = vals[i] * h;
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

      typename U::mac_type t;
      t.assign_zero();
      for (int i=0; i<P.num_players(); i++)
        { t.add(t,tau[i]); }
      if (!t.is_zero()) { throw mac_fail(); }
    }

  vals.erase(vals.begin(), vals.begin() + popen_cnt);
  macs.erase(macs.begin(), macs.begin() + popen_cnt);

  popen_cnt=0;
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
void MAC_Check_Z2k<T, U, V, W>::prepare_open(const W& secret)
{
  shares.push_back(secret.get_share());
  this->values.push_back(V(secret.get_share()));
  this->macs.push_back(secret.get_mac());
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
void MAC_Check_Z2k<T, U, V, W>::set_prep(RandomPrep<W>& prep)
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
Direct_MAC_Check<T>::Direct_MAC_Check(const typename T::mac_key_type::Scalar& ai,
    Names&, int) :
    Direct_MAC_Check<T>(ai)
{
}

template<class T>
Direct_MAC_Check<T>::Direct_MAC_Check(const typename T::mac_key_type::Scalar& ai) :
    MAC_Check_<T>(ai)
{
  open_counter = 0;
}


template<class T>
Direct_MAC_Check<T>::~Direct_MAC_Check() {
  cerr << T::type_string() << " open counter: " << open_counter << endl;
}

template<class T>
void direct_add_openings(vector<T>& values, const PlayerBase& P, vector<octetStream>& os)
{
  for (unsigned int i=0; i<values.size(); i++)
    for (int j=0; j<P.num_players(); j++)
      if (j!=P.my_num())
	values[i].add(os.at(j));
}

template<class T>
void Direct_MAC_Check<T>::exchange(const Player& P)
{
  this->timers[SEND].start();
  P.send_all(this->os,true);
  this->timers[SEND].stop();

  oss.resize(P.num_players());

  this->timers[RECV].start();

  P.receive_all(oss);

  this->timers[RECV].stop();
  open_counter++;

  direct_add_openings<open_type>(this->values, P, oss);

  this->AddToValues(this->values);

  this->popen_cnt += this->values.size();
  this->CheckIfNeeded(P);
}

template<class T>
Passing_MAC_Check<T>::Passing_MAC_Check(const typename T::mac_key_type::Scalar& ai) :
    Direct_MAC_Check<T>(ai)
{
}

template<class T>
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
void Direct_MAC_Check<T>::init_open(const Player& P, int n)
{
  super::init_open(P, n);
  this->os.clear();
  this->os.reserve(n * T::open_type::size());
}

template<class T>
void Direct_MAC_Check<T>::prepare_open(const T& secret)
{
  secret.get_share().pack(this->os);
  this->values.push_back(secret.get_share());
  this->macs.push_back(secret.get_mac());
}

template<class T>
void Passing_MAC_Check<T>::exchange(const Player& P)
{
  for (int i = 0; i < P.num_players() - 1; i++)
    {
      P.pass_around(this->os);
      passing_add_openings(this->values, this->os);
    }
  for (auto& x : this->values)
    x.unpack(this->os);
  this->AddToValues(this->values);
  this->popen_cnt += this->values.size();
  this->CheckIfNeeded(P);
}

#endif
