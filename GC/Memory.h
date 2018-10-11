// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Memory.h
 *
 */

#ifndef GC_MEMORY_H_
#define GC_MEMORY_H_

#include <vector>
#include <sstream>
#include <iostream>
#include <typeinfo>
using namespace std;

#include "Exceptions/Exceptions.h"
#include "Clear.h"

namespace GC
{

template <class T>
class Memory : public vector<T>
{
public:
    void resize(size_t size, const char* name = "");
    void check_index(Integer index) const;
    T& operator[] (Integer i);
    const T& operator[] (Integer i) const;

    template <class U>
    Memory<U>& cast() { return *reinterpret_cast< Memory<U>* >(this); }
};

template <class T>
inline void Memory<T>::check_index(Integer index) const
{
    (void)index;
#ifdef CHECK_SIZE
	size_t i = index.get();
    if (i >= vector<T>::size())
    {
        stringstream ss;
        ss << "Memory overflow: " << i << "/" << vector<T>::size();
        throw Processor_Error(ss.str());
    }
#endif
#ifdef DEBUG_MEMORY
    cout << typeid(T).name() << " at " << this << " index " << i << ": "
            << vector<T>::operator[](i) << endl;
#endif
}

template <class T>
inline T& Memory<T>::operator[] (Integer i)
{
    check_index(i);
    return vector<T>::operator[](i.get());
}

template <class T>
inline const T& Memory<T>::operator[] (Integer i) const
{
    check_index(i);
    return vector<T>::operator[](i.get());
}

} /* namespace GC */

#endif /* GC_MEMORY_H_ */
