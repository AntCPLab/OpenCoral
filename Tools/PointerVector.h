/*
 * PointerVector.h
 *
 */

#ifndef TOOLS_POINTERVECTOR_H_
#define TOOLS_POINTERVECTOR_H_

#include <vector>
using namespace std;

template<class T>
class PointerVector : public vector<T>
{
    int i;

public:
    PointerVector() : i(0) {}
    void clear()
    {
        vector<T>::clear();
        i = 0;
    }
    T& next()
    {
        return (*this)[i++];
    }
};

#endif /* TOOLS_POINTERVECTOR_H_ */
