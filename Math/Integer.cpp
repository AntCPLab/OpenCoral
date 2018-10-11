/*
 * Integer.cpp
 *
 */

#include "Integer.h"

void IntBase::output(ostream& s,bool human) const
{
  if (human)
    s << a;
  else
    s.write((char*)&a, sizeof(a));
}

void IntBase::input(istream& s,bool human)
{
  if (human)
    s >> a;
  else
    s.read((char*)&a, sizeof(a));
}

void to_signed_bigint(bigint& res, const Integer& x, int n)
{
  res = abs(x.get());
  bigint& tmp = bigint::tmp = 1;
  tmp <<= n;
  tmp -= 1;
  res &= tmp;
  if (x < 0)
    res.negate();
}
