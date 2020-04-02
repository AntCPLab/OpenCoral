#include "Processor/Memory.h"
#include "Processor/Instruction.h"

#include <fstream>

template<class T>
void Memory<T>::minimum_size(RegType reg_type, const Program& program, string threadname)
{
  (void) threadname;
  const unsigned* sizes = program.direct_mem(reg_type);
  if (sizes[SECRET] > size_s())
    {
#ifdef DEBUG_MEMORY
      cerr << threadname << " needs more secret " << T::type_string() << " memory, resizing to "
          << sizes[SECRET] << endl;
#endif
      resize_s(sizes[SECRET]);
    }
  if (sizes[CLEAR] > size_c())
    {
#ifdef DEBUG_MEMORY
      cerr << threadname << " needs more clear " << T::type_string() << " memory, resizing to "
          << sizes[CLEAR] << endl;
#endif
      resize_c(sizes[CLEAR]);
    }
}

#ifdef MEMPROTECT
template<class T>
void Memory<T>::protect_s(unsigned int start, unsigned int end)
{
  protected_s.insert(pair<unsigned int,unsigned int>(start, end));
}

template<class T>
void Memory<T>::protect_c(unsigned int start, unsigned int end)
{
  protected_c.insert(pair<unsigned int,unsigned int>(start, end));
}

template<class T>
bool Memory<T>::is_protected_s(unsigned int index)
{
  for (set< pair<unsigned int,unsigned int> >::iterator it = protected_s.begin();
      it != protected_s.end(); it++)
      if (it->first <= index and it->second > index)
        return true;
  return false;
}

template<class T>
bool Memory<T>::is_protected_c(unsigned int index)
{
  for (set< pair<unsigned int,unsigned int> >::iterator it = protected_c.begin();
      it != protected_c.end(); it++)
      if (it->first <= index and it->second > index)
        return true;
  return false;
}
#endif


template<class T>
ostream& operator<<(ostream& s,const Memory<T>& M)
{
  s << M.MS.size() << endl;
  s << M.MC.size() << endl;

#ifdef OUTPUT_HUMAN_READABLE_MEMORY
  for (unsigned int i=0; i<M.MS.size(); i++)
    { M.MS[i].output(s,true); s << endl; }
  s << endl;

  for (unsigned int i=0; i<M.MC.size(); i++)
    {  M.MC[i].output(s,true); s << endl; }
  s << endl;
#else
  for (unsigned int i=0; i<M.MS.size(); i++)
    { M.MS[i].output(s,false); }

  for (unsigned int i=0; i<M.MC.size(); i++)
    { M.MC[i].output(s,false); }
#endif

  return s;
}


template<class T>
istream& operator>>(istream& s,Memory<T>& M)
{
  int len;

  s >> len;  
  M.resize_s(len);
  s >> len;
  M.resize_c(len);
  s.seekg(1, istream::cur);

  for (unsigned int i=0; i<M.MS.size(); i++)
    { M.MS[i].input(s,false);  }

  for (unsigned int i=0; i<M.MC.size(); i++)
    { M.MC[i].input(s,false); }

  return s;
}
