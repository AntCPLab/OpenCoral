
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Share.h"
#include "Auth/fake-stuff.h"
#include "Tools/benchmarking.h"
#include "Processor/config.h"

#include <fstream>

template<class T>
void make_share(vector<Share<T> >& Sa,const T& a,int N,const T& key,PRNG& G)
{
  insecure("share generation", false);
  T mac,x,y;
  mac.mul(a,key);
  Share<T> S;
  S.set_share(a);
  S.set_mac(mac);

  Sa.resize(N);
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
void make_share(FixedVec<T, 2>* Sa, const T& a, int N, PRNG& G);

template<class T>
inline void make_share(vector<T>& Sa,
    const typename T::clear& a, int N, const typename T::value_type& key,
    PRNG& G)
{
  (void)key;
  Sa.resize(N);
  make_share(Sa.data(), a, N, G);
}

template<class T>
void make_share(FixedVec<T, 2>* Sa, const T& a, int N, PRNG& G)
{
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
void check_share(vector<Share<T> >& Sa,T& value,T& mac,int N,const T& key)
{
  value.assign(0);
  mac.assign(0);

  for (int i=0; i<N; i++)
    {
      value.add(Sa[i].get_share());
      mac.add(Sa[i].get_mac());
    }

  T res;
  res.mul(value, key);
  if (!res.equal(mac))
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

template <class T>
void write_mac_keys(const string& directory, int i, int nplayers, gfp macp, T mac2)
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

inline void read_keys(const string& directory, gfp& keyp, gf2n& key2, int nplayers)
{   
    gfp sharep;
    gf2n share2;
    keyp.assign_zero();
    key2.assign_zero();
    int i, tmpN = 0;
    ifstream inpf;

    for (i = 0; i < nplayers; i++)
    {
        stringstream filename;
        filename << directory << "Player-MAC-Keys-P" << i;
        inpf.open(filename.str().c_str());
        if (inpf.fail())
        {
            inpf.close();
            cout << "Error: No MAC key share found for player " << i << std::endl;
            exit(1);
        }
        else
        {
            inpf >> tmpN; // not needed here
            sharep.input(inpf,true);
            share2.input(inpf,true);
            inpf.close();
        }
        std::cout << "    Key " << i << "\t p: " << sharep << "\n\t 2: " << share2 << std::endl;
        keyp.add(sharep);
        key2.add(share2);
    }
    std::cout << "Final MAC keys :\t p: " << keyp << "\n\t\t 2: " << key2 << std::endl;
}
