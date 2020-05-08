/*
 * ShareInterface.h
 *
 */

#ifndef PROTOCOLS_SHAREINTERFACE_H_
#define PROTOCOLS_SHAREINTERFACE_H_

#include <vector>
using namespace std;

class Player;

namespace GC
{
class NoShare;
}

class ShareInterface
{
public:
    typedef GC::NoShare part_type;

    template<class T, class U>
    static void split(vector<U>, vector<int>, int, T*, int, Player&)
    { throw runtime_error("split not implemented"); }

    static bool get_rec_factor(int, int) { return false; }

    template<class T>
    static void read_or_generate_mac_key(const string&, const Names&, T&) {}
};

#endif /* PROTOCOLS_SHAREINTERFACE_H_ */
