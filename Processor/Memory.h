#ifndef _Memory
#define _Memory

/* Class to hold global memory of our system */

#include <iostream>
#include <set>
using namespace std;

// Forward declaration as apparently this is needed for friends in templates
template<class T> class Memory;
template<class T> ostream& operator<<(ostream& s,const Memory<T>& M);
template<class T> istream& operator>>(istream& s,Memory<T>& M);

#include "Processor/Program.h"
#include "Tools/CheckVector.h"

template<class T> 
class Memory
{
#ifdef MEMPROTECT
  set< pair<unsigned int,unsigned int> > protected_s;
  set< pair<unsigned int,unsigned int> > protected_c;
#endif

  public:

  CheckVector<T> MS;
  vector<typename T::clear> MC;

  void resize_s(int sz)
    { MS.resize(sz); }
  void resize_c(int sz)
    { MC.resize(sz); }

  unsigned size_s()
    { return MS.size(); }
  unsigned size_c()
    { return MC.size(); }

  const typename T::clear& read_C(int i) const
    { return MC[i]; }
  const T& read_S(int i) const
    { return MS[i]; }

  void write_C(unsigned int i,const typename T::clear& x,int PC=-1)
    { MC[i]=x;
      (void)PC;
#ifdef MEMPROTECT
    if (is_protected_c(i))
      cerr << "Protected clear memory access of " << i << " by " << PC - 1 << endl;
#endif
    }
  void write_S(unsigned int i,const T& x,int PC=-1)
    { MS[i]=x;
    (void)PC;
#ifdef MEMPROTECT
    if (is_protected_s(i))
      cerr << "Protected secret memory access of " << i << " by " << PC - 1 << endl;
#endif
    }


#ifdef MEMPROTECT
  void protect_s(unsigned int start, unsigned int end);
  void protect_c(unsigned int start, unsigned int end);
  bool is_protected_s(unsigned int index);
  bool is_protected_c(unsigned int index);
#else
  void protect_s(unsigned int start, unsigned int end)
    { (void)start, (void)end; cerr << "Memory protection not activated" << endl; }
  void protect_c(unsigned int start, unsigned int end)
    { (void)start, (void)end; cerr << "Memory protection not activated" << endl; }
#endif

  void minimum_size(RegType reg_type, const Program& program, string threadname);

  friend ostream& operator<< <>(ostream& s,const Memory<T>& M);
  friend istream& operator>> <>(istream& s,Memory<T>& M);
};

#endif

