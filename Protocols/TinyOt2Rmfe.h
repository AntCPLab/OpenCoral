// #ifndef PROTOCOLS_TINYOT2RMFE_H_
// #define PROTOCOLS_TINYOT2RMFE_H_

// #include <memory>
// #include "GC/RmfeShare.h"
// #include "TinyOT/fpre.h"
// #include "TinyOT/tinyot.h"

// class TinyOt2Rmfe {
//     unique_ptr<TinyOTMC> tinyot_mc;
//     unique_ptr<BufferTinyOTPrep> tinyot_prep;
// public:
//     TinyOt2Rmfe(unique_ptr<BufferTinyOTPrep> tinyot_prep) {
//         this->tinyot_mc = unique_ptr<TinyOTMC>(new TinyOTMC());
//         this->tinyot_prep = std::move(tinyot_prep);
//     }

//     BufferTinyOTPrep* get_tinyot_prep() const {
//         return tinyot_prep.get();
//     }

//     TinyOTMC* get_tinyot_mc() const {
//         return tinyot_mc.get();
//     }

//     void convert(std::vector<GC::RmfeShare>& rmfe_shares, const std::vector<TinyOTShare>& tinyot_shares);
// };

// #endif