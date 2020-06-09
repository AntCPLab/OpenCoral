
#include "Math/Setup.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"

#include "Tools/mkpath.h"

#include <fstream>

#include "Math/gfp.hpp"

/*
 * Just setup the primes, doesn't need NTL.
 * Sets idx and m to be used by SHE setup if necessary
 */
bigint SPDZ_Data_Setup_Primes(int lgp)
{
  int idx, m;
  bigint p;
  SPDZ_Data_Setup_Primes(p, lgp, idx, m);
  return p;
}

void SPDZ_Data_Setup_Primes(bigint& p,int lgp,int& idx,int& m)
{
#ifdef VERBOSE
  cout << "Setting up parameters" << endl;
#endif

  switch (lgp)
    { case -1:
        m=16;
        idx=1;    // Any old figures will do, but need to be for lgp at last
        lgp=32;   // Switch to bigger prime to get parameters
        break;
      case 32:
        m=8192;
        idx=0;
        break;
      case 64:
        m=16384;
        idx=1;
        break;
      case 128:
        m=32768; 
        idx=2;
        break;
      case 256: 
        m=32768;
        idx=3;
        break;
      case 512:
        m=65536;
        idx=4;
        break;
      default:
        m=1;
        idx=0;
#ifdef VERBOSE
        cout << "no precomputed parameters, trying anyway" << endl;
#endif
        break;
    }
#ifdef VERBOSE
  cout << "m = " << m << endl;
#endif
  generate_prime(p, lgp, m);
}

void generate_prime(bigint& p, int lgp, int m)
{
  if (OnlineOptions::singleton.prime > 0)
    {
      p = OnlineOptions::singleton.prime;
      if (!probPrime(p))
        {
          cerr << p << " is not a prime" << endl;
          exit(1);
        }
      else if (m != 1 and p % m != 1)
        {
          cerr << p
              << " is not compatible with our encryption scheme, must be 1 modulo "
              << m << endl;
          exit(1);
        }
      else
          return;
    }

  bigint u;
  int ex;
  ex = lgp - numBits(m);
  if (ex < 0)
    throw runtime_error(to_string(lgp) + "-bit primes too small "
            "for our parameters");
  u = 1;
  u = (u << ex) * m;
  p = u + 1;
  while (!probPrime(p) || numBits(p) < lgp)
    {
      u = u + m;
      p = u + 1;
    }

#ifdef VERBOSE
  cout << "\t p = " << p << "  u = " << u << "  :   ";
  cout << lgp << " <= " << numBits(p) << endl;
#endif
}


void generate_online_setup(string dirname, bigint& p, int lgp)
{
  int idx, m;
  SPDZ_Data_Setup_Primes(p, lgp, idx, m);
  write_online_setup(dirname, p);
  gfp::init_field(p);
}

void write_online_setup(string dirname, const bigint& p)
{
  if (p == 0)
    throw runtime_error("prime cannot be 0");

  stringstream ss;
  ss << dirname;
  cout << "Writing to file in " << ss.str() << endl;
  // create preprocessing dir. if necessary
  if (mkdir_p(ss.str().c_str()) == -1)
  {
    cerr << "mkdir_p(" << ss.str() << ") failed\n";
    throw file_error(ss.str());
  }

  // Output the data
  ss << "/Params-Data";
  ofstream outf;
  outf.open(ss.str().c_str());
  outf << p << endl;
}

void init_gf2n(int lg2)
{
  if (lg2 > 64)
    gf2n_long::init_field(lg2);
  else if (lg2 == 0)
    gf2n::init_field(lg2);
  else
    gf2n_short::init_field(lg2);
}
