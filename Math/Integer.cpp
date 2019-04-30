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

void Integer::reqbl(int n)
{
  if ((int)n < 0 && size() * 8 != -(int)n)
    {
      throw Processor_Error(
          "Program compiled for rings of length " + to_string(-(int)n)
          + " but VM supports only "
          + to_string(size() * 8));
    }
  else if ((int)n > 0)
    {
      throw Processor_Error("Program compiled for fields not rings");
    }
}
