
#ifndef _fake_stuff
#define _fake_stuff

#include "Math/gf2n.h"
#include "Math/gfp.h"

#include <fstream>
using namespace std;

template<class T>
void check_share(vector<T>& Sa, typename T::clear& value,
    typename T::value_type& mac, int N, const typename T::value_type& key);

template<class T, class V>
void check_share(vector<Share<T> >& Sa,
  V& value,
  T& mac,
  int N,
  const T& key);

// Generate MAC key shares
void generate_keys(const string& directory, int nplayers);

template <class T, class U>
void write_mac_keys(const string& directory, int player_num, int nplayers, U keyp, T key2);

template <class T, class U>
void read_mac_keys(const string& directory, int player_num, int nplayers, U& keyp, T& key2);

// Read MAC key shares and compute keys
void read_keys(const string& directory, gfp& keyp, gf2n& key2, int nplayers);

template <class T>
class Files
{
public:
  ofstream* outf;
  int N;
  typename T::mac_type key;
  PRNG G;
  Files(int N, const typename T::mac_type& key, const string& prefix) : N(N), key(key)
  {
    outf = new ofstream[N];
    for (int i=0; i<N; i++)
      {
        stringstream filename;
        filename << prefix << "-P" << i;
        cout << "Opening " << filename.str() << endl;
        outf[i].open(filename.str().c_str(),ios::out | ios::binary);
        if (outf[i].fail())
          throw file_error(filename.str().c_str());
      }
    G.ReSeed();
  }
  ~Files()
  {
    delete[] outf;
  }
  void output_shares(const typename T::clear& a)
  {
    vector<T> Sa(N);
    make_share(Sa,a,N,key,G);
    for (int j=0; j<N; j++)
      Sa[j].output(outf[j],false);
  }
};

#endif
