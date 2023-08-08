#ifndef PROTOCOLS_RMFESHARECONVERTER_H_
#define PROTOCOLS_RMFESHARECONVERTER_H_

#include <memory>
#include "GC/RmfeShare.h"
#include "Protocols/ProtocolGlobalInit.h"

template<class T>
class RmfeShareConverter {
    typedef typename T::MAC_Check SrcMC;
    typedef typename T::LivePrep SrcLivePrep;

public:
    BinaryProtocolGlobalInit<T> binit;
    SrcMC* src_mc;
    SrcLivePrep* src_prep;

    RmfeShareConverter(SrcMC* src_mc, SrcLivePrep* src_prep) {
        this->src_mc = src_mc;
        this->src_prep = src_prep;
    }

    /**
     * Assuming ShareThread singleton has been set, we take out the MC and prep from there.
    */
    RmfeShareConverter(Player& P): binit(P) {
        GC::ShareThread<T>& thread = GC::ShareThread<T>::s();
        this->src_mc = thread.MC;
        this->src_prep = dynamic_cast<SrcLivePrep*>(&thread.DataF);
    }

    ~RmfeShareConverter() {
    }

    SrcLivePrep* get_src_prep() const {
        return src_prep;
    }

    SrcMC* get_src_mc() const {
        return src_mc;
    }

    void convert(std::vector<GC::RmfeShare>& rmfe_shares, const std::vector<T>& src_shares);
};

#endif