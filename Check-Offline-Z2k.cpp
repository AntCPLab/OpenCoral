
#include "Math/Z2k.h"
#include "Protocols/Share.h"
#include "Math/Setup.h"
#include "Protocols/Spdz2kShare.h"
#include "Protocols/fake-stuff.h"

#include "Protocols/fake-stuff.hpp"
#include "Math/Z2k.hpp"

#include <fstream>
#include <vector>
#include <numeric>

template <class W>
void check_triples_Z2k(int n_players, string type_char = "")
{
    typedef typename W::open_type T;
    typedef typename W::mac_key_type U;
    typedef typename W::open_type V;

    T keyp; keyp.assign_zero();
    U pp;
    ifstream inpf;
    for (int i= 0; i < n_players; i++)
    {
      stringstream ss;
      ss << get_prep_dir(n_players, 128, 128) << "Player-MAC-Keys";
      ss << "-P" << i;
      cout << "Opening file " << ss.str() << endl;
      inpf.open(ss.str().c_str());
      if (inpf.fail()) { throw file_error(ss.str()); }
      int n;
      inpf >> n;
      assert(n == n_players);
      pp.input(inpf,true);
      cout << " Key " << i << "\t p: " << pp << endl;
      keyp.add(pp);
      inpf.close();
    }
    cout << "--------------\n";
    cout << "Final Keys :\t p: " << keyp << endl;

    ifstream* inputFiles = new ifstream[n_players];
    for (int i = 0; i < n_players; i++)
    {
        stringstream ss;
        ss << get_prep_dir(n_players, 128, 128) << "Triples-";
        if (type_char.size())
            ss << type_char;
        else
            ss << W::type_short();
        ss << "-P" << i;
        inputFiles[i].open(ss.str().c_str());
        cout << "Opening file " << ss.str() << endl;
    }

    int j = 0;
    while (inputFiles[0].peek() != EOF)
    {
        V a,b,c,prod;
        T mac;
        vector<Share<T>> as(n_players), bs(n_players), cs(n_players);

        for (int i = 0; i < n_players; i++)
        {
          as[i].input(inputFiles[i], false);
          bs[i].input(inputFiles[i], false);
          cs[i].input(inputFiles[i], false);
        }

        check_share<T, V>(as, a, mac, n_players, keyp);
        check_share<T, V>(bs, b, mac, n_players, keyp);
        check_share<T, V>(cs, c, mac, n_players, keyp);
        
        prod.mul(a, b);
        if (prod != c)
        {
          cout << j << ": " << c << " != " << a << " * " << b << endl;
          throw bad_value();
        }
        j++;
    }

    cout << dec << j << " correct triples of type " << T::type_string() << endl;
    delete[] inputFiles;
}

template<int K, int S>
void check(int n_players)
{
    check_triples_Z2k<Spdz2kShare<K, S>>(n_players);
}

int main(int argc, char** argv)
{
    int n_players = 2;
    if (argc > 1)
        n_players = atoi(argv[1]);
    int k = 64;
    if (argc > 2)
    {
        k = atoi(argv[2]);
    }
    int s = k;
    if (argc > 3)
        s = atoi(argv[3]);

    if (k == 32)
        check<32, 32>(n_players);
    else if (k == 64 and s == 64)
        check<64, 64>(n_players);
    else if (k == 64 and s == 48)
        check<64, 48>(n_players);
    else if (k == 66 and s == 64)
        check<66, 64>(n_players);
    else if (k == 66 and s == 48)
        check<66, 48>(n_players);
    else
        throw runtime_error("not compiled for k=" + to_string(k) + ", s=" + to_string(s));
}
