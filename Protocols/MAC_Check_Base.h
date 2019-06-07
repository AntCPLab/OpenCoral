/*
 * MAC_Check_Base.h
 *
 */

#ifndef PROTOCOLS_MAC_CHECK_BASE_H_
#define PROTOCOLS_MAC_CHECK_BASE_H_

#include <vector>
using namespace std;

#include "Networking/Player.h"

template<class T>
class MAC_Check_Base
{
protected:
    /* MAC Share */
    typename T::mac_key_type alphai;

public:
    int values_opened;

    MAC_Check_Base() : values_opened(0) {}
    virtual ~MAC_Check_Base() {}

    virtual void Check(const Player& P) { (void)P; }

    int number() const { return values_opened; }

    const typename T::mac_key_type& get_alphai() const { return alphai; }

    virtual void POpen_Begin(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) = 0;
    virtual void POpen_End(vector<typename T::open_type>& values,const vector<T>& S,const Player& P) = 0;
    void POpen(vector<typename T::open_type>& values,const vector<T>& S,const Player& P);
    typename T::open_type POpen(const T& secret, const Player& P);

    virtual void CheckFor(const typename T::open_type& value, const vector<T>& shares, const Player& P);
};

#endif /* PROTOCOLS_MAC_CHECK_BASE_H_ */
