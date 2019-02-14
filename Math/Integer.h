/*
 * Integer.h
 *
 */

#ifndef INTEGER_H_
#define INTEGER_H_

#include <iostream>
using namespace std;

#include "Tools/octetStream.h"
#include "Tools/random.h"
#include "bigint.h"
#include "field_types.h"


// Functionality shared between integers and bit vectors
class IntBase
{
protected:
  long a;

public:
  static int size() { return sizeof(a); }
  static string type_string() { return "integer"; }

  static void init_default(int lgp) { (void)lgp; }

  static bool allows(Dtype type) { return type <= DATA_BIT; }

  IntBase()                 { a = 0; }
  IntBase(long a) : a(a)    {}

  long get() const          { return a; }
  bool get_bit(int i) const { return (a >> i) & 1; }

  unsigned long debug() const { return a; }

  void assign(long x)       { *this = x; }
  void assign(const char* buffer) { avx_memcpy(&a, buffer, sizeof(a)); }
  void assign_zero()        { a = 0; }
  void assign_one()         { a = 1; }

  bool is_zero() const      { return a == 0; }
  bool is_one() const       { return a == 1; }
  bool is_bit() const       { return is_zero() or is_one(); }

  long operator>>(const IntBase& other) const { return a >> other.a; }
  long operator<<(const IntBase& other) const { return a << other.a; }

  long operator^(const IntBase& other) const { return a ^ other.a; }
  long operator&(const IntBase& other) const { return a & other.a; }
  long operator|(const IntBase& other) const { return a | other.a; }

  bool operator==(const IntBase& other) const { return a == other.a; }
  bool operator!=(const IntBase& other) const { return a != other.a; }

  bool equal(const IntBase& other) const { return *this == other; }

  long operator^=(const IntBase& other) { return a ^= other.a; }
  long operator&=(const IntBase& other) { return a &= other.a; }

  friend ostream& operator<<(ostream& s, const IntBase& x) { x.output(s, true); return s; }

  void randomize(PRNG& G);

  void output(ostream& s,bool human) const;
  void input(istream& s,bool human);

  void pack(octetStream& os) const { os.store_int(a, sizeof(a)); }
  void unpack(octetStream& os) { a = os.get_int(sizeof(a)); }
};

// Wrapper class for integer
class Integer : public IntBase
{
  public:

  typedef Integer value_type;
  typedef Integer clear;

  static char type_char() { return 'R'; }
  static DataFieldType field_type() { return DATA_INT; }

  static void reqbl(int n);

  Integer()                 { a = 0; }
  Integer(long a) : IntBase(a) {}
  Integer(const bigint& x)  { *this = x.get_si(); }

  void convert_destroy(bigint& other) { *this = other.get_si(); }

  long operator+(const Integer& other) const { return a + other.a; }
  long operator-(const Integer& other) const { return a - other.a; }
  long operator*(const Integer& other) const { return a * other.a; }
  long operator/(const Integer& other) const { return a / other.a; }

  bool operator<(const Integer& other) const { return a < other.a; }
  bool operator<=(const Integer& other) const { return a <= other.a; }
  bool operator>(const Integer& other) const { return a > other.a; }
  bool operator>=(const Integer& other) const { return a >= other.a; }

  long operator+=(const Integer& other) { return a += other.a; }

  friend unsigned int& operator+=(unsigned int& x, const Integer& other) { return x += other.a; }

  long operator-() const { return -a; }

  void add(const Integer& x, const Integer& y) { *this = x + y; }
  void sub(const Integer& x, const Integer& y) { *this = x - y; }
  void mul(const Integer& x, const Integer& y) { *this = x * y; }

  void mul(const Integer& x) { *this = *this * x; }

  void invert() { throw runtime_error("cannot invert integer"); }
  void invert(const Integer& _) { (void)_; invert(); }

  void AND(const Integer& x, const Integer& y) { *this = x & y; }
  void OR(const Integer& x, const Integer& y) { *this = x | y; }
  void XOR(const Integer& x, const Integer& y) { *this = x ^ y; }
  void SHL(const Integer& x, const Integer& y) { *this = x << y; }
  // unsigned shift for Mod2m
  void SHR(const Integer& x, const Integer& y) { *this = (unsigned long)x.a >> y.a; }

  template <class T, class U>
  void add(const T&, const U&) { throw not_implemented(); }
};

inline void IntBase::randomize(PRNG& G)
{
  a = G.get_word();
}

inline void to_bigint(bigint& res, const Integer& x)
{
  res = (unsigned long)x.get();
}

inline void to_signed_bigint(bigint& res, const Integer& x)
{
  res = x.get();
}

void to_signed_bigint(bigint& res, const Integer& x, int n);

// slight misnomer
inline void to_gfp(Integer& res, const bigint& x)
{
  res = x.get_si();
}

#endif /* INTEGER_H_ */
