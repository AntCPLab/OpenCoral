
#ifndef _Share
#define _Share

/* Class for holding a share of either a T or gfp element */ 

#include <vector>
#include <iostream>
using namespace std;

#include "Math/gf2n.h"
#include "Protocols/SPDZ.h"

// Forward declaration as apparently this is needed for friends in templates
template<class T> class Share;
template<class T> T combine(const vector< Share<T> >& S);
template<class T> bool check_macs(const vector< Share<T> >& S,const T& key);

template<class T> class MAC_Check_;
template<class T> class Direct_MAC_Check;
template<class T> class MascotMultiplier;
template<class T> class MascotFieldPrep;
template<class T> class MascotTripleGenerator;
template<class T> class MascotPrep;

union square128;

template<class T>
class Share
{
   T a;        // The share
   T mac;      // Shares of the mac

   public:

   typedef T mac_key_type;
   typedef T mac_type;
   typedef T open_type;
   typedef T clear;

   typedef Share<typename T::next> prep_type;
   typedef MascotMultiplier<Share> Multiplier;
   typedef MascotTripleGenerator<prep_type> TripleGenerator;
   typedef T sacri_type;
   typedef typename T::Square Rectangle;
   typedef Rectangle Square;

   typedef MAC_Check_<Share> MAC_Check;
   typedef Direct_MAC_Check<Share> Direct_MC;
   typedef ::Input<Share> Input;
   typedef ::PrivateOutput<Share> PrivateOutput;
   typedef SPDZ<Share> Protocol;
   typedef MascotFieldPrep<Share> LivePrep;
   typedef MascotPrep<Share> RandomPrep;

   const static bool needs_ot = true;
   const static bool dishonest_majority = true;

   static int size()
     { return 2 * T::size(); }

   static string type_string()
     { return "SPDZ " + T::type_string(); }

   static string type_short()
     { return string(1, T::type_char()); }

   static char type_char()
     { return T::type_char(); }

   static DataFieldType field_type()
     { return T::field_type(); }

   static int threshold(int nplayers)
     { return nplayers - 1; }

   static Share constant(const clear& aa, int my_num, const typename T::Scalar& alphai)
     { return Share(aa, my_num, alphai); }

   template<class U>
   void assign(const Share<U>& S)
     { a=S.get_share(); mac=S.get_mac(); }
   void assign(const char* buffer)
     { a.assign(buffer); mac.assign(buffer + T::size()); }
   void assign_zero()
     { a.assign_zero(); 
       mac.assign_zero(); 
     }
   void assign(const clear& aa, int my_num, const typename T::Scalar& alphai);

   Share()                  { assign_zero(); }
   template<class U>
   Share(const Share<U>& S) { assign(S); }
   Share(const clear& aa, int my_num, const typename T::Scalar& alphai)
     { assign(aa, my_num, alphai); }
   Share(const T& share, const T& mac) : a(share), mac(mac) {}
   ~Share()                 { ; }

   const T& get_share() const          { return a; }
   const T& get_mac() const            { return mac; }
   void set_share(const T& aa)  { a=aa; }
   void set_mac(const T& aa)    { mac=aa; }

   /* Arithmetic Routines */
   void mul(const Share<T>& S,const T& aa);
   void mul_by_bit(const Share<T>& S,const T& aa);
   void add(const Share<T>& S,const clear& aa,int my_num,const T& alphai);
   void negate() { a.negate(); mac.negate(); }
   void sub(const Share<T>& S,const clear& aa,int my_num,const T& alphai);
   void sub(const clear& aa,const Share<T>& S,int my_num,const T& alphai);
   void add(const Share<T>& S1,const Share<T>& S2);
   void sub(const Share<T>& S1,const Share<T>& S2);
   void add(const Share<T>& S1) { add(*this,S1); }

   // obsolete interface
   void add(const Share<T>& S,const clear& aa,bool playerone,const T& alphai);
   void sub(const Share<T>& S,const clear& aa,bool playerone,const T& alphai);
   void sub(const clear& aa,const Share<T>& S,bool playerone,const T& alphai);

   Share<T> operator+(const Share<T>& x) const
   { Share<T> res; res.add(*this, x); return res; }
   Share<T> operator-(const Share<T>& x) const
   { Share<T> res; res.sub(*this, x); return res; }
   template <class U>
   Share<T> operator*(const U& x) const
   { Share<T> res; res.mul(*this, x); return res; }
   Share<T> operator/(const T& x) const
   { Share<T> res; res.set_share(a / x); res.set_mac(mac / x); return res; }

   Share<T>& operator+=(const Share<T>& x) { add(x); return *this; }
   template <class U>
   Share<T>& operator*=(const U& x) { mul(*this, x); return *this; }

   Share<T> operator<<(int i) { return this->operator*(T(1) << i); }
   Share<T>& operator<<=(int i) { return *this = *this << i; }

   Share<T> operator>>(int i) { return {a >> i, mac >> i}; }

   void force_to_bit() { a.force_to_bit(); }

   // Input and output from a stream
   //  - Can do in human or machine only format (later should be faster)
   void output(ostream& s,bool human) const
     { a.output(s,human);     if (human) { s << " "; }
       mac.output(s,human);
     }
   void input(istream& s,bool human)
     { a.input(s,human);
       mac.input(s,human);
     }

   friend ostream& operator<<(ostream& s, const Share<T>& x) { x.output(s, true); return s; }

   void pack(octetStream& os, bool full = true) const;
   void unpack(octetStream& os, bool full = true);

    /* Takes a vector of shares, one from each player and
     * determines the shared value
     *   - i.e. Partially open the shares
     */
   friend T combine<T>(const vector< Share<T> >& S);

   /* Given a set of shares, one from each player and
    * the global key, determines if the sharing is valid
    *   - Mainly for test purposes
    */
   friend bool check_macs<T>(const vector< Share<T> >& S,const T& key);
};

template <class T, class U, class V>
using Share_ = Share<T>;

// specialized mul by bit for gf2n
template <>
void Share<gf2n>::mul_by_bit(const Share<gf2n>& S,const gf2n& aa);

template <class T>
Share<T> operator*(const T& y, const Share<T>& x) { Share<T> res; res.mul(x, y); return res; }

template<class T>
inline void Share<T>::add(const Share<T>& S1,const Share<T>& S2)
{
  a.add(S1.a,S2.a);
  mac.add(S1.mac,S2.mac);
}

template<class T>
void Share<T>::sub(const Share<T>& S1,const Share<T>& S2)
{
  a.sub(S1.a,S2.a);
  mac.sub(S1.mac,S2.mac);
}

template<class T>
inline void Share<T>::mul(const Share<T>& S,const T& aa)
{
  a.mul(S.a,aa);
  mac.mul(S.mac,aa);
}

template<class T>
inline void Share<T>::add(const Share<T>& S,const clear& aa,int my_num,const T& alphai)
{
  *this = S + Share<T>(aa, my_num, alphai);
}

template<class T>
inline void Share<T>::sub(const Share<T>& S,const clear& aa,int my_num,const T& alphai)
{
  *this = S - Share<T>(aa, my_num, alphai);
}

template<class T>
inline void Share<T>::sub(const clear& aa,const Share<T>& S,int my_num,const T& alphai)
{
  *this = Share<T>(aa, my_num, alphai) - S;
}

template<class T>
inline void Share<T>::assign(const clear& aa, int my_num,
    const typename T::Scalar& alphai)
{
  Protocol::assign(a, aa, my_num);
  mac.mul(aa, alphai);
#ifdef DEBUG_MAC
  cout << "load " << hex << mac << " = " << aa << " * " << alphai << endl;
#endif
}

#endif
