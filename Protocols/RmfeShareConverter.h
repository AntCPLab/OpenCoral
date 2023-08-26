#ifndef PROTOCOLS_RMFESHARECONVERTER_H_
#define PROTOCOLS_RMFESHARECONVERTER_H_

#include <memory>
#include "GC/RmfeShare.h"
#include "Protocols/ProtocolGlobalInit.h"

template<class T>
class RmfeShareConverter {
    typedef typename T::MAC_Check SrcMC;
    typedef typename T::LivePrep SrcLivePrep;
    typedef typename GC::RmfeShare::MAC_Check DstMC;

public:
    SrcMC* src_mc;
    SrcLivePrep* src_prep;

    // RmfeShareConverter(SrcMC* src_mc, SrcLivePrep* src_prep) {
    //     this->src_mc = src_mc;
    //     this->src_prep = src_prep;
    // }

    /**
     * Assuming ShareThread singleton has been set, we take out the MC and prep from there.
    */
    RmfeShareConverter(Player& P) {
        GC::ShareThread<T>& thread = GC::ShareThread<T>::s();
        this->src_mc = thread.MC;
        this->src_prep = dynamic_cast<SrcLivePrep*>(&thread.DataF);

        // [zico] Why create a new mc here? Usually we cannot directly use the MC from ShareThread because
        // our outer application will probaly use it to open values. If we use it here, and convert is used
        // in the middle of outer openning, then the MC open inside convert will destroy the outer openning.
        // It is a general problem for nested openning with the same MC.
        // We use ShareThread for src_mc above because that mc is only expected to be used here. If we intend to
        // use it in outer application, then the application code should be carefully designed to not introduce the nested
        // openning for src_mc.
        // dst_mc = GC::RmfeShare::new_mc(
        //     GC::ShareThread<GC::RmfeShare::whole_type>::s().MC->get_alphai());
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