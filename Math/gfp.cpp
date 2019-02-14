
#include "Math/gfp.h"
#include "Math/Setup.h"

#include "Exceptions/Exceptions.h"

Zp_Data gfp::ZpD;

void gfp::init_default(int lgp)
{
  init_field(SPDZ_Data_Setup_Primes(lgp));
}

void gfp::almost_randomize(PRNG& G)
{
  G.get_octets((octet*)a.x,t()*sizeof(mp_limb_t));
  a.x[t()-1]&=ZpD.mask;
}

void gfp::AND(const gfp& x,const gfp& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_and(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

void gfp::OR(const gfp& x,const gfp& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_ior(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

void gfp::XOR(const gfp& x,const gfp& y)
{
  bigint bi1,bi2;
  to_bigint(bi1,x);
  to_bigint(bi2,y);
  mpz_xor(bi1.get_mpz_t(), bi1.get_mpz_t(), bi2.get_mpz_t());
  convert_destroy(bi1);
}

void gfp::AND(const gfp& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_and(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}

void gfp::OR(const gfp& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_ior(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}

void gfp::XOR(const gfp& x,const bigint& y)
{
  bigint bi;
  to_bigint(bi,x);
  mpz_xor(bi.get_mpz_t(), bi.get_mpz_t(), y.get_mpz_t());
  convert_destroy(bi);
}




void gfp::SHL(const gfp& x,int n)
{
  if (!x.is_zero())
    {
      if (n != 0)
        {
          bigint& bi = bigint::tmp;
          to_bigint(bi,x,false);
          bi <<= n;
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


void gfp::SHR(const gfp& x,int n)
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


void gfp::SHL(const gfp& x,const bigint& n)
{
  SHL(x,mpz_get_si(n.get_mpz_t()));
}


void gfp::SHR(const gfp& x,const bigint& n)
{
  SHR(x,mpz_get_si(n.get_mpz_t()));
}


gfp gfp::sqrRoot()
{
    // Temp move to bigint so as to call sqrRootMod
    bigint ti;
    to_bigint(ti, *this);
    ti = sqrRootMod(ti, ZpD.pr);
    if (!isOdd(ti))
        ti = ZpD.pr - ti;
    gfp temp;
    to_gfp(temp, ti);
    return temp;
}

void gfp::reqbl(int n)
{
  if ((int)n > 0 && gfp::pr() < bigint(1) << (n-1))
    {
      cout << "Tape requires prime of bit length " << n << endl;
      throw invalid_params();
    }
  else if ((int)n < 0)
    {
      throw Processor_Error("Program compiled for rings not fields");
    }
}

bool gfp::allows(Dtype type)
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

void to_signed_bigint(bigint& ans, const gfp& x)
{
    to_bigint(ans, x);
    // get sign and abs(x)
    bigint& p_half = bigint::tmp = (gfp::pr()-1)/2;
    if (mpz_cmp(ans.get_mpz_t(), p_half.get_mpz_t()) > 0)
        ans = gfp::pr() - ans;
}
