#include "Math/Setup.h"
#include "Auth/fake-stuff.hpp"
#include <stdlib.h>
#include <fstream>
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 4)
    { cout << "Call using\n\t";
      cout << "Setup.x n lgp lg2 \n";
      cout << "\t\t n           = Number of players" << endl;
      cout << "\t\t lgp         = Bit size of char p message space" << endl;
      cout << "\t\t lg2         = Bit size of char 2 message space" << endl;
      exit(1);
    }

  int n=atoi(argv[1]);
  int lgp=atoi(argv[2]);
  int lg2=atoi(argv[3]);

  string dir = get_prep_dir(n, lgp, lg2);
  ofstream outf;
  bigint p;
  generate_online_setup(outf, dir, p, lgp, lg2);

  bool need_mac = false;
  for (int i = 0; i < n; i++)
    {
      string filename = mac_filename(dir, i);
      ifstream in(filename);
      need_mac |= not in.good();
    }
  if (need_mac)
    generate_keys(dir, n);
}


