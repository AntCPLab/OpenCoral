// (C) 2018 University of Bristol. See License.txt

#include "Math/Z2k.h"
#include "Math/Share.h"
#include "Math/Setup.h"
#include "Auth/fake-stuff.h"

#include <fstream>
#include <vector>
#include <numeric>

template <class T, class U, class V>
void check_triples_Z2k(int n_players, string type_char = "")
{
    T keyp; keyp.assign_zero();
    U pp;
    ifstream inpf;
    for (int i= 0; i < n_players; i++)
    {
      stringstream ss;
      ss << get_prep_dir(n_players, 128, 128) << "Player-MAC-Key-";
      if (type_char.size())
          ss << type_char;
      else
          ss << U::type_char();
      ss << "-P" << i;
      cout << "Opening file " << ss.str() << endl;
      inpf.open(ss.str().c_str());
      if (inpf.fail()) { throw file_error(ss.str()); }
      pp.input(inpf,false);
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
            ss << T::type_char();
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

int main(int argc, char** argv)
{
    int n_players = 2;
    if (argc > 1)
        n_players = atoi(argv[1]);
    check_triples_Z2k<Z2<SPDZ2K_K + SPDZ2K_S>, Z2<SPDZ2K_S>, Z2<SPDZ2K_K>>(n_players);
}
