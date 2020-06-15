/*
 * gf2nlong.h
 *
 */

#ifndef MATH_GF2NLONG_H_
#define MATH_GF2NLONG_H_

#include <stdlib.h>
#include <string.h>

#include <iostream>
using namespace std;

#include <smmintrin.h>

#include "Tools/random.h"
#include "Math/field_types.h"
#include "Math/bigint.h"
#include "Math/Bit.h"


class int128
{
public:
    __m128i a;

    int128() : a(_mm_setzero_si128()) { }
    int128(const __m128i& a) : a(a) { }
    int128(const word& a) : a(_mm_cvtsi64_si128(a)) { }
    int128(const word& upper, const word& lower) : a(_mm_set_epi64x(upper, lower)) { }

    word get_lower() const                      { return (word)_mm_cvtsi128_si64(a); }
    word get_upper() const     { return _mm_cvtsi128_si64(_mm_unpackhi_epi64(a, a)); }
    word get_half(bool upper) const { return upper ? get_upper() : get_lower(); }

#ifdef __SSE41__
    bool operator==(const int128& other) const  { return _mm_test_all_zeros(a ^ other.a, a ^ other.a); }
#else
    bool operator==(const int128& other) const  { return get_lower() == other.get_lower() and get_upper() == other.get_upper(); }
#endif
    bool operator!=(const int128& other) const  { return !(*this == other); }

    int128 operator<<(const int& other) const;
    int128 operator>>(const int& other) const;

    int128 operator^(const int128& other) const { return a ^ other.a; }
    int128 operator|(const int128& other) const { return a | other.a; }
    int128 operator&(const int128& other) const { return a & other.a; }

    int128 operator~() const                    { return ~a; }

    int128& operator<<=(const int& other)       { return *this = *this << other; }
    int128& operator>>=(const int& other)       { return *this = *this >> other; }

    int128& operator^=(const int128& other)     { a ^= other.a; return *this; }
    int128& operator|=(const int128& other)     { a |= other.a; return *this; }
    int128& operator&=(const int128& other)     { a &= other.a; return *this; }

    friend ostream& operator<<(ostream& s, const int128& a);

    static int128 ones(int n);
    bool get_bit(int i) const;
};


template<class T> class Input;
template<class T> class PrivateOutput;
template<class T> class SPDZ;
template<class T> class Share;
template<class T> class Square;

namespace GC
{
class NoValue;
}

/* This interface compatible with the gfp interface
 * which then allows us to template the Share
 * data type.
 */


/*
  Arithmetic in Gf_{2^n} with n<=128
*/

class gf2n_long : public ValueInterface
{
  int128 a;

  static int n,t1,t2,t3,nterms;
  static int l0,l1,l2,l3;
  static int128 mask,lowermask,uppermask;

  /* Assign x[0..2*nwords] to a and reduce it...  */
  void reduce_trinomial(int128 xh,int128 xl);
  void reduce_pentanomial(int128 xh,int128 xl);

  public:

  typedef gf2n_long value_type;
  typedef int128 internal_type;

  typedef gf2n_long next;
  typedef ::Square<gf2n_long> Square;

  const static int MAX_N_BITS = 128;
  const static int N_BYTES = sizeof(a);

  static const int N_BITS = -1;

  typedef gf2n_long Scalar;

  void reduce(int128 xh,int128 xl)
   {
     if (nterms==3)
        { reduce_pentanomial(xh,xl); }
     else
        { reduce_trinomial(xh,xl);   }
   }

  static void init_field(int nn);
  static int degree() { return n; }
  static int length() { return n; }
  static int default_degree() { return 128; }
  static int get_nterms() { return nterms; }
  static int get_t(int i)
    { if (i==0)      { return t1; }
      else if (i==1) { return t2; }
      else if (i==2) { return t3; }
      return -1;
    }

  static DataFieldType field_type() { return DATA_GF2N; }
  static char type_char() { return '2'; }
  static string type_string() { return "gf2n_long"; }

  static int size() { return sizeof(a); }
  static int size_in_bits() { return sizeof(a) * 8; }
  static int t()    { return 0; }

  static int default_length() { return 128; }

  static bool allows(Dtype type) { (void) type; return true; }

  static const bool invertible = true;
  static const bool characteristic_two = true;

  static gf2n_long cut(int128 x) { return x; }

  static gf2n_long Mul(gf2n_long a, gf2n_long b) { return a * b; }

  int128 get() const { return a; }
  word get_word() const { return _mm_cvtsi128_si64(a.a); }

  const void* get_ptr() const { return &a.a; }

  void assign(const gf2n_long& g)     { a=g.a; }

  void assign_zero()             { a=_mm_setzero_si128(); }
  void assign_one()              { a=int128(0,1); }
  void assign_x()                { a=int128(0,2); }
  void assign(int128 aa)           { a=aa&mask; }
  void assign(int aa)            { a=int128(static_cast<unsigned int>(aa))&mask; }
  void assign(const void* buffer) { a = _mm_loadu_si128((__m128i*)buffer); }

  void normalize() {}

  int get_bit(int i) const
    { return ((a>>i)&1).get_lower(); }
  void set_bit(int i,unsigned int b)
  { if (b==1)
      { a |= (1UL<<i); }
    else
      { a &= ~(1UL<<i); }
  }

  gf2n_long()              { assign_zero(); }
  gf2n_long(const gf2n_long& g) { assign(g); }
  gf2n_long(const int128& g)    { assign(g); }
  gf2n_long(int g)         { assign(g); }
  template<class T>
  gf2n_long(IntBase<T> g)       { assign(g.get()); }
  gf2n_long(const char* buffer) { assign(buffer); }
  gf2n_long(GC::NoValue);
  ~gf2n_long()             { ; }

  gf2n_long& operator=(const gf2n_long& g)
    { if (&g!=this) { assign(g); }
      return *this;
    }

  int is_zero() const            { return a==int128(0); }
  int is_one()  const            { return a==int128(1); }
  int equal(const gf2n_long& y) const { return (a==y.a); }
  bool operator==(const gf2n_long& y) const { return a==y.a; }
  bool operator!=(const gf2n_long& y) const { return a!=y.a; }

  // x+y
  void add(const gf2n_long& x,const gf2n_long& y)
    { a=x.a^y.a; }
  void add(const gf2n_long& x)
    { a^=x.a; }
  void add(octet* x)
    { a^=int128(_mm_loadu_si128((__m128i*)x)); }
  void add(octetStream& os)
    { add(os.consume(size())); }
  void sub(const gf2n_long& x,const gf2n_long& y)
    { a=x.a^y.a; }
  void sub(const gf2n_long& x)
    { a^=x.a; }
  // = x * y
  gf2n_long& mul(const gf2n_long& x,const gf2n_long& y);
  void mul(const gf2n_long& x) { mul(*this,x); }
  // x * y when one of x,y is a bit
  void mul_by_bit(const gf2n_long& x, const gf2n_long& y)   { a = x.a.a * y.a.a; }

  gf2n_long operator+(const gf2n_long& x) const { gf2n_long res; res.add(*this, x); return res; }
  gf2n_long operator*(const gf2n_long& x) const { gf2n_long res; res.mul(*this, x); return res; }
  gf2n_long& operator+=(const gf2n_long& x) { add(x); return *this; }
  gf2n_long& operator*=(const gf2n_long& x) { mul(x); return *this; }
  gf2n_long operator-(const gf2n_long& x) const { gf2n_long res; res.add(*this, x); return res; }
  gf2n_long& operator-=(const gf2n_long& x) { sub(x); return *this; }
  gf2n_long operator/(const gf2n_long& x) const { gf2n_long tmp; tmp.invert(x); return *this * tmp; }

  void square();
  void square(const gf2n_long& aa);
  void invert();
  void invert(const gf2n_long& aa)
    { *this=aa; invert(); }
  void negate() { return; }
  void power(long i);

  /* Bitwise Ops */
  void AND(const gf2n_long& x,const gf2n_long& y) { a=x.a&y.a; }
  void XOR(const gf2n_long& x,const gf2n_long& y) { a=x.a^y.a; }
  void OR(const gf2n_long& x,const gf2n_long& y)  { a=x.a|y.a; }
  void NOT(const gf2n_long& x)               { a=(~x.a)&mask; }
  void SHL(const gf2n_long& x,int n)         { a=(x.a<<n)&mask; }
  void SHR(const gf2n_long& x,int n)         { a=x.a>>n; }

  gf2n_long operator&(const gf2n_long& x) const { gf2n_long res; res.AND(*this, x); return res; }
  gf2n_long operator^(const gf2n_long& x) const { gf2n_long res; res.XOR(*this, x); return res; }
  gf2n_long operator|(const gf2n_long& x) const { gf2n_long res; res.OR(*this, x); return res; }
  gf2n_long operator!() const { gf2n_long res; res.NOT(*this); return res; }
  gf2n_long operator<<(int i) const { gf2n_long res; res.SHL(*this, i); return res; }
  gf2n_long operator>>(int i) const { gf2n_long res; res.SHR(*this, i); return res; }

  gf2n_long& operator&=(const gf2n_long& x) { AND(*this, x); return *this; }
  gf2n_long& operator>>=(int i) { SHR(*this, i); return *this; }
  gf2n_long& operator<<=(int i) { SHL(*this, i); return *this; }

  /* Crap RNG */
  void randomize(PRNG& G, int n = -1);
  // compatibility with gfp
  void almost_randomize(PRNG& G)        { randomize(G); }

  void force_to_bit() { a &= 1; }

  template<class T>
  T convert() const { return *this; }

  void output(ostream& s,bool human) const;
  void input(istream& s,bool human);

  friend ostream& operator<<(ostream& s,const gf2n_long& x)
    { s << hex << x.a << dec;
      return s;
    }
  friend istream& operator>>(istream& s,gf2n_long& x)
    { bigint tmp;
      s >> hex >> tmp >> dec;
      x.a = 0;
      mpn_copyi((word*)&x.a.a, tmp.get_mpz_t()->_mp_d, tmp.get_mpz_t()->_mp_size);
      return s;
    }


  // Pack and unpack in native format
  //   i.e. Dont care about conversion to human readable form
  void pack(octetStream& o, int n = -1) const
    { (void) n; o.append((octet*) &a,sizeof(__m128i)); }
  void unpack(octetStream& o, int n = -1)
    { (void) n; o.consume((octet*) &a,sizeof(__m128i)); }
};


inline int128 int128::operator<<(const int& other) const
{
  int128 res(_mm_slli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask = _mm_srli_epi64(a, 64 - other);
  else
    mask = _mm_slli_epi64(a, other - 64);
  res.a ^= _mm_slli_si128(mask, 8);
  return res;
}

inline int128 int128::operator>>(const int& other) const
{
  int128 res(_mm_srli_epi64(a, other));
  __m128i mask;
  if (other < 64)
    mask = _mm_slli_epi64(a, 64 - other);
  else
    mask = _mm_srli_epi64(a, other - 64);
  res.a ^= _mm_srli_si128(mask, 8);
  return res;
}

void mul64(word x, word y, word& lo, word& hi);

inline __m128i software_clmul(__m128i a, __m128i b, int choice)
{
    word lo, hi;
    mul64(int128(a).get_half(choice & 1),
            int128(b).get_half((choice & 0x10) >> 4), lo, hi);
    return int128(hi, lo).a;
}

template<int choice>
inline __m128i clmul(__m128i a, __m128i b)
{
#ifdef __PCLMUL__
    if (cpu_has_pclmul())
    {
        return _mm_clmulepi64_si128(a, b, choice);
    }
    else
#endif
        return software_clmul(a, b, choice);
}

inline void mul128(__m128i a, __m128i b, __m128i *res1, __m128i *res2)
{
    __m128i tmp3, tmp4, tmp5, tmp6;

    tmp3 = clmul<0x00>(a, b);
    tmp4 = clmul<0x10>(a, b);
    tmp5 = clmul<0x01>(a, b);
    tmp6 = clmul<0x11>(a, b);

    tmp4 = _mm_xor_si128(tmp4, tmp5);
    tmp5 = _mm_slli_si128(tmp4, 8);
    tmp4 = _mm_srli_si128(tmp4, 8);
    tmp3 = _mm_xor_si128(tmp3, tmp5);
    tmp6 = _mm_xor_si128(tmp6, tmp4);
    // initial mul now in tmp3, tmp6
    *res1 = tmp3;
    *res2 = tmp6;
}

inline int128 int128::ones(int n)
{
    if (n < 64)
        return int128(0, (1ULL << n) - 1);
    else
        return int128((1ULL << (n - 64)) - 1, -1);
}

inline bool int128::get_bit(int i) const
{
    if (i < 64)
        return (get_lower() >> i) & 1;
    else
        return (get_upper() >> (i - 64)) & 1;
}

inline gf2n_long& gf2n_long::mul(const gf2n_long& x,const gf2n_long& y)
{
  __m128i res[2];
  memset(res,0,sizeof(res));

  mul128(x.a.a,y.a.a,res,res+1);

  reduce(res[1],res[0]);

  return *this;
}

#endif /* MATH_GF2NLONG_H_ */
