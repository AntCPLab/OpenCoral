#ifndef PROTOCOLS_TINYOT2RMFE_H_
#define PROTOCOLS_TINYOT2RMFE_H_

#include "GC/RmfeShare.h"
#include "TinyOT/fpre.h"
#include "TinyOT/tinyot.h"

class TinyOt2Rmfe {
    TinyOTMC* tinyot_mc;
    BufferTinyOTPrep* tinyot_prep;
public:
    TinyOt2Rmfe(TinyOTMC* mc, BufferTinyOTPrep* prep) {
        this->tinyot_mc = mc;
        this->tinyot_prep = prep;
    }

    void convert(std::vector<GC::RmfeShare>& rmfe_shares, const std::vector<TinyOTShare>& tinyot_shares);
};

#endif