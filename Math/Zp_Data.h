#ifndef _Zp_Data
#define _Zp_Data

/* Class to define helper information for a Zp element
 *
 * Basically the data needed for Montgomery operations 
 *
 * Almost all data is public as this is basically a container class
 *
 */

#include "Math/config.h"
#include "Math/bigint.h"
#include "Math/mpn_fixed.h"
#include "Tools/random.h"

#include <smmintrin.h>
#include <iostream>
using namespace std;

#ifndef MAX_MOD_SZ
   #ifdef LargeM
     #define MAX_MOD_SZ 20
   #else
     #define MAX_MOD_SZ 2
  #endif
#endif

class modp;

class Zp_Data
{
  bool        montgomery;  // True if we are using Montgomery arithmetic
  mp_limb_t   R[MAX_MOD_SZ],R2[MAX_MOD_SZ],R3[MAX_MOD_SZ],pi;
  // extra limb needed for Montgomery multiplication
  mp_limb_t   prA[MAX_MOD_SZ+1];
  int         t;           // More Montgomery data

  template <int T>
  void Mont_Mult_(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;
  void Mont_Mult(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;
  void Mont_Mult_variable(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const;

  public:

  bigint       pr;
  mp_limb_t    mask;
  size_t       pr_byte_length;

  void assign(const Zp_Data& Zp);
  void init(const bigint& p,bool mont=true);
  int get_t() const { return t; }
  const mp_limb_t* get_prA() const { return prA; }

  void pack(octetStream& o) const;
  void unpack(octetStream& o);

  // This one does nothing, needed so as to make vectors of Zp_Data
  Zp_Data() : montgomery(0), pi(0), mask(0), pr_byte_length(0) { t=MAX_MOD_SZ; }

  // The main init funciton
  Zp_Data(const bigint& p,bool mont=true)
    { init(p,mont); }

  Zp_Data(const Zp_Data& Zp) { assign(Zp); }
  Zp_Data& operator=(const Zp_Data& Zp) 
    { if (this!=&Zp) { assign(Zp); }
      return *this;
    }
  ~Zp_Data()  { ; }

  template <int T>
  void Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  void Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  template <int T>
  void Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;
  void Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const;

  __m128i get_random128(PRNG& G);

  bool operator!=(const Zp_Data& other) const;

   friend void to_bigint(bigint& ans,const modp& x,const Zp_Data& ZpD,bool reduce);

   friend void to_modp(modp& ans,int x,const Zp_Data& ZpD);
   friend void to_modp(modp& ans,const bigint& x,const Zp_Data& ZpD);

   friend void Add(modp& ans,const modp& x,const modp& y,const Zp_Data& ZpD);
   friend void Sub(modp& ans,const modp& x,const modp& y,const Zp_Data& ZpD);
   friend void Mul(modp& ans,const modp& x,const modp& y,const Zp_Data& ZpD);
   friend void Sqr(modp& ans,const modp& x,const Zp_Data& ZpD);
   friend void Negate(modp& ans,const modp& x,const Zp_Data& ZpD);
   friend void Inv(modp& ans,const modp& x,const Zp_Data& ZpD);

   friend void Power(modp& ans,const modp& x,int exp,const Zp_Data& ZpD);
   friend void Power(modp& ans,const modp& x,const bigint& exp,const Zp_Data& ZpD);

   friend void assignOne(modp& x,const Zp_Data& ZpD);
   friend void assignZero(modp& x,const Zp_Data& ZpD);
   friend bool isZero(const modp& x,const Zp_Data& ZpD);
   friend bool isOne(const modp& x,const Zp_Data& ZpD);
   friend bool areEqual(const modp& x,const modp& y,const Zp_Data& ZpD);

   friend class modp;

   friend ostream& operator<<(ostream& s,const Zp_Data& ZpD);
   friend istream& operator>>(istream& s,Zp_Data& ZpD);
};

template<>
inline void Zp_Data::Add<0>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t carry = mpn_add_n(ans,x,y,t);
  if (carry!=0 || mpn_cmp(ans,prA,t)>=0)
    { mpn_sub_n(ans,ans,prA,t); }
}

template<>
inline void Zp_Data::Add<1>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
#ifdef __clang__
  Add<0>(ans, x, y);
#else
  *ans = *x + *y;
  asm goto ("jc %l[sub]" :::: sub);
  if (*ans >= *prA)
 sub:
      *ans -= *prA;
#endif
}

template<>
inline void Zp_Data::Add<2>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
#ifdef __clang__
  Add<0>(ans, x, y);
#else
  __uint128_t a, b, p;
  memcpy(&a, x, sizeof(__uint128_t));
  memcpy(&b, y, sizeof(__uint128_t));
  memcpy(&p, prA, sizeof(__uint128_t));
  __uint128_t c = a + b;
  asm goto ("jc %l[sub]" :::: sub);
  if (c >= p)
 sub:
      c -= p;
  memcpy(ans, &c, sizeof(__uint128_t));
#endif
}

inline void Zp_Data::Add(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  switch (t)
  {
  case 2:
    return Add<2>(ans, x, y);
  case 1:
    return Add<1>(ans, x, y);
  default:
    return Add<0>(ans, x, y);
  }
}

template <int T>
inline void Zp_Data::Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t tmp[T];
  mp_limb_t borrow = mpn_sub_fixed_n_borrow<T>(tmp, x, y);
  if (borrow != 0)
    mpn_add_fixed_n<T>(ans, tmp, prA);
  else
    inline_mpn_copyi(ans, tmp, T);
}

template <>
inline void Zp_Data::Sub<0>(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t borrow = mpn_sub_n(ans,x,y,t);
  if (borrow!=0)
    mpn_add_n(ans,ans,prA,t);
}

inline void Zp_Data::Sub(mp_limb_t* ans,const mp_limb_t* x,const mp_limb_t* y) const
{
  switch (t)
  {
  case 2:
    Sub<2>(ans, x, y);
    break;
  case 1:
    Sub<1>(ans, x, y);
    break;
  default:
    Sub<0>(ans, x, y);
    break;
  }
}

#ifdef __BMI2__
template <int T>
inline void Zp_Data::Mont_Mult_(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const
{
  mp_limb_t ans[2*MAX_MOD_SZ+1],u;
  inline_mpn_zero(ans + T + 1, T);
  // First loop
  u=x[0]*y[0]*pi;
  mpn_mul_1_fixed<T + 1, T>(ans,y,x[0]);
  mpn_addmul_1_fixed_<T + 2, T + 1>(ans,prA,u);
  for (int i=1; i<T; i++)
    { // u=(ans0+xi*y0)*pd
      u=(ans[i]+x[i]*y[0])*pi;
      // ans=ans+xi*y+u*pr
      mpn_addmul_1_fixed_<T + 1, T>(ans+i,y,x[i]);
      mpn_addmul_1_fixed_<T + 2, T + 1>(ans+i,prA,u);
    }
  // if (ans>=pr) { ans=z-pr; }
  // else         { z=ans;    }
  if (mpn_cmp(ans+T,prA,T+1)>=0)
     { mpn_sub_fixed_n<T>(z,ans+T,prA); }
  else
     { inline_mpn_copyi(z,ans+T,T); }
}
#endif

inline void Zp_Data::Mont_Mult(mp_limb_t* z,const mp_limb_t* x,const mp_limb_t* y) const
{
  switch (t)
  {
#ifdef __BMI2__
  case 2:
    Mont_Mult_<2>(z, x, y);
    break;
  case 1:
    Mont_Mult_<1>(z, x, y);
    break;
#endif
  default:
    Mont_Mult_variable(z, x, y);
    break;
  }
}

#endif
