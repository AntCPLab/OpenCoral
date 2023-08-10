/*
 * Spdz2kBShare.h
 *
 */

#ifndef GC_SPDZ2KBSHARE_H_
#define GC_SPDZ2KBSHARE_H_

#include "TinySecret.h"

namespace GC {
    template<int S>
    using Spdz2kBShare = GC::TinySecret<S>;
}

// #include "ShareSecret.h"
// #include "ShareParty.h"
// #include "Secret.h"
// #include "Protocols/Spdz2kShare.h"


// namespace GC
// {

// template<int S> class TinySecret;
// template<class T> class ShareThread;
// template<class T> class TinierSharePrep;

// /**
//  * This is "almost" a copy of TinyShare. But the difference is:
//  * When using TinyShare, there are many places that assume it is wrapped as a part of TinySecret, e.g., 
//  * in the usage of ShareThread, the `get_party` funciton of TinyShare actually returns ShareThread<TinySecret>.
//  * 
//  * Whereas, Spdz2kBShare removes this assumption and can be used as a standalone type.
//  *  
// */
// template<int S>
// class Spdz2kBShare : public Spdz2kShare<1, S>, public ShareSecret<Spdz2kBShare<S>>
// {
//     typedef Spdz2kBShare This;

// public:
//     typedef Spdz2kShare<1, S> super;

//     typedef void DynamicMemory;

//     typedef Beaver<This> Protocol;
//     typedef MAC_Check_Z2k_<This> MAC_Check;
//     typedef MAC_Check Direct_MC;
//     typedef MAC_Check MC;
//     typedef ::Input<This> Input;
//     typedef TinierSharePrep<This> LivePrep;

//     typedef SwitchableOutput out_type;

//     typedef This small_type;

//     typedef NoShare bit_type;

//     // `is_real` exists in multiple base classes (ShareSecret and ShareInterface)
//     static const bool is_real = true;

//     static string name()
//     {
//         return "spdz2k B share";
//     }

//     Spdz2kBShare()
//     {
//     }
//     Spdz2kBShare(const typename super::super::super& other) :
//             super(other)
//     {
//     }
//     Spdz2kBShare(const super& other) :
//             super(other)
//     {
//     }

//     void XOR(const This& a, const This& b)
//     {
//         *this = a + b;
//     }

//     void public_input(bool input)
//     {
//         auto& party = this->get_party();
//         *this = super::constant(input, party.P->my_num(),
//                 party.MC->get_alphai());
//     }

//     static MAC_Check* new_mc(typename super::mac_key_type mac_key)
//     {
//         return new MAC_Check(mac_key);
//     }
// };

// } /* namespace GC */

#endif /* GC_SPDZ2KBSHARE_H_ */
