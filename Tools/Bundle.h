/*
 * Bundle.h
 *
 */

#ifndef TOOLS_BUNDLE_H_
#define TOOLS_BUNDLE_H_

#include <vector>
using namespace std;

template<class T>
class Bundle : public vector<T>
{
public:
    T& mine;

    Bundle(const PlayerBase& P) :
            vector<T>(P.num_players()), mine(this->at(P.my_num()))
    {
    }
};

#endif /* TOOLS_BUNDLE_H_ */
