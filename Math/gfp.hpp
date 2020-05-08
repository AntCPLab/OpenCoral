#ifndef MATH_GFP_HPP_
#define MATH_GFP_HPP_

#include "Math/gfp.h"
#include "Math/Setup.h"

#include "Exceptions/Exceptions.h"

#include "Math/bigint.hpp"
#include "Math/Setup.hpp"

template<int X, int L>
inline void gfp_<X, L>::read_or_generate_setup(string dir,
        const OnlineOptions& opts)
{
  if (opts.prime == 0)
    read_setup<gfp_<X, L>>(dir, opts.lgp);
  else
    init_field(opts.prime);
}

template<int X, int L>
void gfp_<X, L>::init_field(const bigint& p, bool mont)
{
  ZpD.init(p, mont);
  string name = "gfp<" + to_string(X) + ", " + to_string(L) + ">";
  if (ZpD.get_t() > L)
    {
      cout << "modulus is " << p << endl;
      throw wrong_gfp_size(name + " too small for modulus. "
              "Maybe change GFP_MOD_SZ to " + to_string(ZpD.get_t()));
    }
  if (ZpD.get_t() < L)
    {
      if (mont)
        throw wrong_gfp_size(name + " too large for modulus. "
            "Maybe change GFP_MOD_SZ to " + to_string(ZpD.get_t()));
      else
        cerr << name << " larger than necessary for modulus " << p << endl;
    }
}

template <int X, int L>
void gfp_<X, L>::init_default(int lgp, bool mont)
{
  init_field(SPDZ_Data_Setup_Primes(lgp), mont);
}

template <int X, int L>
void gfp_<X, L>::check()
{
  assert(mpn_cmp(a.x, ZpD.get_prA(), t()) < 0);
}

template <int X, int L>
void gfp_<X, L>::almost_randomize(PRNG& G)
{
  G.get_octets((octet*)a.x,t()*sizeof(mp_limb_t));
  a.x[t()-1]&=ZpD.mask;
}

template <int X, int L>
void gfp_<X, L>::AND(const gfp_& x,const gfp_& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_and(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

template <int X, int L>
void gfp_<X, L>::OR(const gfp_& x,const gfp_& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_ior(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

template <int X, int L>
void gfp_<X, L>::XOR(const gfp_& x,const gfp_& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_xor(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

template <int X, int L>
void gfp_<X, L>::AND(const gfp_& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_and(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}

template <int X, int L>
void gfp_<X, L>::OR(const gfp_& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_ior(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}

template <int X, int L>
void gfp_<X, L>::XOR(const gfp_& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_xor(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}




template <int X, int L>
void gfp_<X, L>::SHL(const gfp_& x,int n)
{
  if (!x.is_zero())
    {
      if (n != 0)
        {
          *this = x * power_of_two(1, n);
        }
      else
        assign(x);
    }
  else
    {
      assign_zero();
    }
}


template <int X, int L>
void gfp_<X, L>::SHR(const gfp_& x,int n)
{
  if (!x.is_zero())
    {
      if (n != 0)
        {
          bigint& bi = bigint::tmp;
          to_bigint(bi,x);
          bi >>= n;
          convert_destroy(bi);
        }
      else
        assign(x);
    }
  else
    {
      assign_zero();
    }
}


template <int X, int L>
void gfp_<X, L>::SHL(const gfp_& x,const bigint& n)
{
  SHL(x,mpz_get_si(n.get_mpz_t()));
}


template <int X, int L>
void gfp_<X, L>::SHR(const gfp_& x,const bigint& n)
{
  SHR(x,mpz_get_si(n.get_mpz_t()));
}


template<int X, int L>
gfp_<X, L> gfp_<X, L>::sqrRoot()
{
    // Temp move to bigint so as to call sqrRootMod
    bigint ti;
    to_bigint(ti, *this);
    ti = sqrRootMod(ti, ZpD.pr);
    if (!isOdd(ti))
        ti = ZpD.pr - ti;
    gfp_<X, L> temp;
    to_gfp(temp, ti);
    return temp;
}

template <int X, int L>
void gfp_<X, L>::reqbl(int n)
{
  if ((int)n > 0 && pr() < bigint(1) << (n-1))
    {
      cout << "Tape requires prime of bit length " << n << endl;
      throw invalid_params();
    }
  else if ((int)n < 0)
    {
      throw Processor_Error("Program compiled for rings not fields");
    }
}

template<int X, int L>
bool gfp_<X, L>::allows(Dtype type)
{
    switch(type)
    {
    case DATA_BITGF2NTRIPLE:
    case DATA_BITTRIPLE:
        return false;
    default:
        return true;
    }
}

template<int X, int L>
void gfp_<X, L>::specification(octetStream& os)
{
    os.store(pr());
}

template <int X, int L>
gfp_<X, L> gfp_<X, L>::power_of_two(bool bit, int exp)
{
    if (bit)
    {
        while (exp >= int(powers.size()))
        {
            bigint::tmp = 1;
            bigint::tmp <<= powers.size();
            powers.push_back(bigint::tmp);
        }
        return powers.at(exp);
    }
    else
        return 0;
}

#endif
