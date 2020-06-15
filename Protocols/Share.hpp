
#include "Share.h"


template<class T>
template<class U>
void Share<T>::read_or_generate_mac_key(string directory, const Names& N,
        U& key)
{
    try
    {
        read_mac_key(directory, N, key);
    }
    catch (mac_key_error&)
    {
#ifdef VERBOSE
        cerr << "Generating fresh MAC key for " << type_string() << endl;
#endif
        SeededPRNG G;
        key.randomize(G);
    }
}

template<class T, class V>
inline
void Share_<T, V>::mul_by_bit(const Share_<T, V>& S,const clear& aa)
{
  a.mul(S.a,aa);
  mac.mul(S.mac,aa);
}

template<>
inline
void Share_<SemiShare<gf2n>, SemiShare<gf2n>>::mul_by_bit(
    const Share_<SemiShare<gf2n>, SemiShare<gf2n>>& S, const gf2n& aa)
{
  a.mul_by_bit(S.a,aa);
  mac.mul_by_bit(S.mac,aa);
}

template<class T, class V>
inline void Share_<T, V>::pack(octetStream& os, bool full) const
{
  a.pack(os);
  if (full)
    mac.pack(os);
}

template<class T, class V>
inline void Share_<T, V>::unpack(octetStream& os, bool full)
{
  a.unpack(os);
  if (full)
    mac.unpack(os);
}
