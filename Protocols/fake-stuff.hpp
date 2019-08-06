
#include "Protocols/fake-stuff.h"
#include "Processor/Data_Files.h"
#include "Tools/benchmarking.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include <fstream>

template<class T> class Share;
template<class T> class SemiShare;
template<class T, int L> class FixedVec;

template<class T, class U, class V>
void make_share(Share<T>* Sa,const U& a,int N,const V& key,PRNG& G)
{
  insecure("share generation", false);
  T mac,x,y;
  mac.mul(a,key);
  Share<T> S;
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

template<class T>
void make_share(FixedVec<T, 2>* Sa, const T& a, int N, const T& key, PRNG& G);

template<class T>
inline void make_share(vector<T>& Sa,
    const typename T::clear& a, int N, const typename T::mac_type& key,
    PRNG& G)
{
  Sa.resize(N);
  make_share(Sa.data(), a, N, key, G);
}

template<class T>
void make_share(FixedVec<T, 2>* Sa, const T& a, int N, const T& key, PRNG& G)
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

inline void generate_keys(const string& directory, int nplayers)
{
  PRNG G;
  G.ReSeed();

  gf2n mac2;
  gfp macp;
  mac2.assign_zero();
  macp.assign_zero();

  for (int i = 0; i < nplayers; i++)
  {
    mac2.randomize(G);
    macp.randomize(G);
    write_mac_keys(directory, i, nplayers, macp, mac2);
  }
}

inline string mac_filename(string directory, int playerno)
{
  if (directory.empty())
    directory = ".";
  return directory + "/Player-MAC-Keys-P" + to_string(playerno);
}

template <class T, class U>
void write_mac_keys(const string& directory, int i, int nplayers, U macp, T mac2)
{
  ofstream outf;
  stringstream filename;
  filename << mac_filename(directory, i);
  cout << "Writing to " << filename.str().c_str() << endl;
  outf.open(filename.str().c_str());
  outf << nplayers << endl;
  macp.output(outf,true);
  outf << " ";
  mac2.output(outf,true);
  outf << endl;
  outf.close();
}

template <class T, class U>
void read_mac_keys(const string& directory, const Names& N, U& keyp, T& key2)
{
  read_mac_keys(directory, N.my_num(), N.num_players(), keyp, key2);
}

template <class T, class U>
void read_mac_keys(const string& directory, int player_num, int nplayers, U& keyp, T& key2)
{
  int nn;

  string filename = directory + "Player-MAC-Keys-P" + to_string(player_num);
  ifstream inpf;
  inpf.open(filename);
  if (inpf.fail())
    {
#ifdef VERBOSE
      cerr << "Could not open MAC key file. Perhaps it needs to be generated?\n";
#endif
      throw file_error(filename);
    }
  inpf >> nn;
  if (nn!=nplayers)
    { cerr << "KeyGen was last run with " << nn << " players." << endl;
      cerr << "  - You are running Online with " << nplayers << " players." << endl;
      exit(1);
    }

  keyp.input(inpf,true);
  key2.input(inpf,true);
  inpf.close();
}

template<class T>
void generate_mac_keys(typename T::mac_key_type::Scalar& keyp, gf2n& key2,
    int nplayers, string prep_data_prefix)
{
  keyp.assign_zero();
  gf2n p2;
  key2.assign_zero();
  int tmpN = 0;
  ifstream inpf;
  SeededPRNG G;
  for (int i = 0; i < nplayers; i++)
    {
      stringstream filename;
      filename << prep_data_prefix << "Player-MAC-Keys-P" << i;
      inpf.open(filename.str().c_str());
      typename T::mac_key_type::Scalar pp;
      gf2n p2;
      if (inpf.fail())
        {
          inpf.close();
          cout << "No MAC key share for player " << i << ", generating a fresh one\n";
          pp.randomize(G);
          p2.randomize(G);
          ofstream outf(filename.str().c_str());
          if (outf.fail())
            throw file_error(filename.str().c_str());
          outf << nplayers << " " << pp << " " << p2;
          outf.close();
          cout << "Written new MAC key share to " << filename.str() << endl;
        }
      else
        {
          inpf >> tmpN; // not needed here
          pp.input(inpf,true);
          p2.input(inpf,true);
          inpf.close();
        }
      cout << " Key " << i << "\t p: " << pp << "\n\t 2: " << p2 << endl;
      keyp.add(pp);
      key2.add(p2);
    }
  cout << "--------------\n";
  cout << "Final Keys :\t p: " << keyp << "\n\t\t 2: " << key2 << endl;
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
      filename << prep_data_prefix << "Triples-" << T::type_short() << "-P" << i
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
      filename << prep_data_prefix << "Inverses-" << T::type_short() << "-P" << i;
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
