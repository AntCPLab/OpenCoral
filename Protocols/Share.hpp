
#include "Share.h"
#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Math/Z2k.h"
#include "Math/FixedVec.h"
#include "Math/Integer.h"


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
T combine(const vector< Share<T> >& S)
{
  T ans=S[0].a;
  for (unsigned int i=1; i<S.size(); i++) 
    { ans.add(ans,S[i].a); }
  return ans;
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


template<class T, class V>
bool check_macs(const vector< Share<T> >& S,const T& key)
{
  T val=combine(S);

  // Now check the MAC is valid
  val.mul(val,key);
  for (unsigned i=0; i<S.size(); i++)
    { val.sub(val,S[i].mac); }
  if (!val.is_zero()) { return false; }
  return true;
}
