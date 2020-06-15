#ifndef PROTOCOLS_FAKE_STUFF_HPP_
#define PROTOCOLS_FAKE_STUFF_HPP_

#include "Protocols/fake-stuff.h"
#include "Processor/Data_Files.h"
#include "Tools/benchmarking.h"
#include "Math/Setup.h"

#include "Protocols/ShamirInput.hpp"

#include <fstream>

template<class T> class Share;
template<class T> class SemiShare;
template<class T> class ShamirShare;
template<class T, int L> class FixedVec;
template<class T, class V> class Share_;

namespace GC
{
template<int S> class TinySecret;
template<class T> class TinierSecret;
}

template<class T, class U, class V, class W>
void make_share(Share_<T, W>* Sa,const U& a,int N,const V& key,PRNG& G)
{
  insecure("share generation", false);
  T x;
  W mac, y;
  mac = a * key;
  Share_<T, W> S;
  S.set_share(a);
  S.set_mac(mac);

  for (int i=0; i<N-1; i++)
    { x.randomize(G);
      y.randomize(G);
      Sa[i].set_share(x);
      Sa[i].set_mac(y);
      S.sub(S,Sa[i]);
    }
  Sa[N-1]=S;
}

template<class T, class U, class V>
void make_vector_share(T* Sa,const U& a,int N,const V& key,PRNG& G)
{
  int length = Sa[0].default_length;
  for (int i = 0; i < N; i++)
    Sa[i].resize_regs(length);
  for (int j = 0; j < length; j++)
    {
      typename T::part_type shares[N];
      make_share(shares, typename T::part_type::clear(a.get_bit(j)), N, key, G);
      for (int i = 0; i < N; i++)
        Sa[i].get_reg(j) = shares[i];
    }
}

template<int S, class U, class V>
void make_share(GC::TinySecret<S>* Sa, const U& a, int N, const V& key, PRNG& G)
{
  make_vector_share(Sa, a, N, key, G);
}

template<class T, class U, class V>
void make_share(GC::TinierSecret<T>* Sa, const U& a, int N, const V& key, PRNG& G)
{
  make_vector_share(Sa, a, N, key, G);
}

template<class T>
void make_share(SemiShare<T>* Sa,const T& a,int N,const T& key,PRNG& G)
{
  (void) key;
  insecure("share generation", false);
  T x, S = a;
  for (int i=0; i<N-1; i++)
    {
      x.randomize(G);
      Sa[i] = x;
      S -= x;
    }
  Sa[N-1]=S;
}

template<class T, class U, class V>
void make_share(FixedVec<T, 2>* Sa, const V& a, int N, const U& key, PRNG& G);

template<class T>
inline void make_share(vector<T>& Sa,
    const typename T::clear& a, int N, const typename T::mac_type& key,
    PRNG& G)
{
  Sa.resize(N);
  make_share(Sa.data(), a, N, key, G);
}

template<class T, class U, class V>
void make_share(FixedVec<T, 2>* Sa, const V& a, int N, const U& key, PRNG& G)
{
  (void) key;
  assert(N == 3);
  insecure("share generation", false);
  FixedVec<T, 3> add_shares;
  // hack
  add_shares.randomize_to_sum(a, G);
  for (int i=0; i<N; i++)
    {
      FixedVec<T, 2> share;
      share[0] = add_shares[(i + 1) % 3];
      share[1] = add_shares[i];
      Sa[i] = share;
    }
}

template<class T>
void make_share(ShamirShare<T>* Sa, const T& a, int N,
    const typename ShamirShare<T>::mac_type&, PRNG& G)
{
  insecure("share generation", false);
  const auto& vandermonde = ShamirInput<ShamirShare<T>>::get_vandermonde(N / 2, N);
  vector<T> randomness(N / 2);
  for (auto& x : randomness)
      x.randomize(G);
  for (int i = 0; i < N; i++)
  {
      auto& share = Sa[i];
      share = a;
      for (int j = 0; j < N / 2; j++)
          share += vandermonde[i][j] * randomness[j];
  }
}

template<class T, class V>
void check_share(vector<Share<T> >& Sa,
  V& value,
  T& mac,
  int N,
  const T& key)
{
  value.assign(0);
  mac.assign(0);

  for (int i=0; i<N; i++)
    {
      value.add(Sa[i].get_share());
      mac.add(Sa[i].get_mac());
    }

  V res;
  res.mul(value, key);
  if (res != mac)
    {
      cout << "Value:      " << value << endl;
      cout << "Input MAC:  " << mac << endl;
      cout << "Actual MAC: " << res << endl;
      cout << "MAC key:    " << key << endl;
      throw mac_fail();
    }
}

template<class T>
void check_share(vector<T>& Sa, typename T::clear& value,
    typename T::value_type& mac, int N, const typename T::value_type& key)
{
  assert(N == 3);
  value = 0;
  (void)key;
  (void)mac;

  for (int i = 0; i < N; i++)
    {
      auto share = Sa[i];
      value += share[0];
      auto a = share[1];
      auto b = Sa[positive_modulo(i - 1, N)][0];
      if (a != b)
      {
        cout << a << " != " << b << endl;
        cout << hex << a.debug() << " != " << b.debug() << endl;
        for (int i = 0; i < N; i++)
          cout << Sa[i] << endl;
        throw bad_value("invalid replicated secret sharing");
      }
    }
}

template<class T>
inline string mac_filename(string directory, int playerno)
{
  if (directory.empty())
    directory = ".";
  return directory + "/Player-MAC-Keys-" + string(1, T::type_char()) + "-P"
      + to_string(playerno);
}

template <class U>
void write_mac_key(const string& directory, int i, int nplayers, U key)
{
  ofstream outf;
  stringstream filename;
  filename << mac_filename<U>(directory, i);
  cout << "Writing to " << filename.str().c_str() << endl;
  outf.open(filename.str().c_str());
  outf << nplayers << endl;
  key.output(outf,true);
  outf.close();
}

template <class T>
void read_mac_key(const string& directory, const Names& N, T& key)
{
  read_mac_key(directory, N.my_num(), N.num_players(), key);
}

template <class U>
void read_mac_key(const string& directory, int player_num, int nplayers, U& key)
{
  int nn;

  string filename = mac_filename<U>(directory, player_num);
  ifstream inpf;
#ifdef VERBOSE
  cerr << "Reading MAC keys from " << filename << endl;
#endif
  inpf.open(filename);
  if (inpf.fail())
    {
#ifdef VERBOSE
      cerr << "Could not open MAC key file. Perhaps it needs to be generated?\n";
#endif
      throw mac_key_error(filename);
    }
  inpf >> nn;
  if (nn!=nplayers)
    { cerr << "KeyGen was last run with " << nn << " players." << endl;
      cerr << "  - You are running Online with " << nplayers << " players." << endl;
      throw mac_key_error(filename);
    }

  key.input(inpf,true);

  if (inpf.fail())
      throw mac_key_error(filename);

  inpf.close();
}

template <class U>
void read_global_mac_key(const string& directory, int nparties, U& key)
{
  U pp;
  key.assign_zero();

  for (int i= 0; i < nparties; i++)
    {
      read_mac_key(directory, i, nparties, pp);
      cout << " Key " << i << ": " << pp << endl;
      key.add(pp);
    }

  cout << "--------------\n";
  cout << "Final Keys : " << key << endl;
}

template<class T>
void generate_mac_keys(typename T::mac_type::Scalar& key,
    int nplayers, string prep_data_prefix)
{
  key.assign_zero();
  int tmpN = 0;
  ifstream inpf;
  SeededPRNG G;
  prep_data_prefix = get_prep_sub_dir<T>(prep_data_prefix, nplayers);

  for (int i = 0; i < nplayers; i++)
    {
      stringstream filename;
      filename
          << mac_filename<typename T::mac_key_type::Scalar>(prep_data_prefix,
              i);
      inpf.open(filename.str().c_str());
      typename T::mac_key_type::Scalar pp;
      if (inpf.fail())
        {
          inpf.close();
          cout << "No MAC key share for player " << i << ", generating a fresh one\n";
          pp.randomize(G);
          ofstream outf(filename.str().c_str());
          if (outf.fail())
            throw file_error(filename.str().c_str());
          outf << nplayers << " " << pp << endl;
          outf.close();
          cout << "Written new MAC key share to " << filename.str() << endl;
        }
      else
        {
          inpf >> tmpN; // not needed here
          pp.input(inpf,true);
          inpf.close();
        }
      cout << " Key " << i << ": " << pp << endl;
      key.add(pp);
    }
  cout << "--------------\n";
  cout << "Final Key: " << key << endl;
}

/* N      = Number players
 * ntrip  = Number triples needed
 * str    = "2" or "p"
 */
template<class T>
void make_mult_triples(const typename T::mac_type& key, int N, int ntrip,
    bool zero, string prep_data_prefix, int thread_num = -1)
{
  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  typename T::clear a,b,c;
  vector<T> Sa(N),Sb(N),Sc(N);
  /* Generate Triples */
  for (int i=0; i<N; i++)
    { stringstream filename;
      filename << get_prep_sub_dir<T>(prep_data_prefix, N) << "Triples-" << T::type_short() << "-P" << i
          << Sub_Data_Files<T>::get_suffix(thread_num);
      cout << "Opening " << filename.str() << endl;
      outf[i].open(filename.str().c_str(),ios::out | ios::binary);
      if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
    }
  for (int i=0; i<ntrip; i++)
    {
      if (!zero)
        a.randomize(G);
      make_share(Sa,a,N,key,G);
      if (!zero)
        b.randomize(G);
      make_share(Sb,b,N,key,G);
      c.mul(a,b);
      make_share(Sc,c,N,key,G);
      for (int j=0; j<N; j++)
        { Sa[j].output(outf[j],false);
          Sb[j].output(outf[j],false);
          Sc[j].output(outf[j],false);
        }
    }
  for (int i=0; i<N; i++)
    { outf[i].close(); }
  delete[] outf;
}

/* N      = Number players
 * ntrip  = Number inverses needed
 * str    = "2" or "p"
 */
template<class T>
void make_inverse(const typename T::mac_type& key, int N, int ntrip, bool zero,
    string prep_data_prefix)
{
  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  typename T::clear a,b;
  vector<T> Sa(N),Sb(N);
  /* Generate Triples */
  for (int i=0; i<N; i++)
    { stringstream filename;
      filename << get_prep_sub_dir<T>(prep_data_prefix, N) << "Inverses-" << T::type_short() << "-P" << i;
      cout << "Opening " << filename.str() << endl;
      outf[i].open(filename.str().c_str(),ios::out | ios::binary);
      if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
    }
  for (int i=0; i<ntrip; i++)
    {
      if (zero)
        // ironic?
        a.assign_one();
      else
        do
          a.randomize(G);
        while (a.is_zero());
      make_share(Sa,a,N,key,G);
      b=a; b.invert();
      make_share(Sb,b,N,key,G);
      for (int j=0; j<N; j++)
        { Sa[j].output(outf[j],false);
          Sb[j].output(outf[j],false);
        }
    }
  for (int i=0; i<N; i++)
    { outf[i].close(); }
  delete[] outf;
}

#endif
